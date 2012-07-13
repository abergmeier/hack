
#include <algorithm>
#include <vector>
#include <string>
#include <chrono>
#include <exception>
#include <iostream>
#include <sstream>
#include <tuple>
#include <string.h> //for strnlen
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/AutoPtr.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketAcceptor.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/NObserver.h>
#include <Poco/Net/NetException.h>

#include "network.hpp"

using namespace hack::net;

namespace {
	struct Debug : public hack::Debug {
		const std::string& GetCategory() const override {
			static const std::string CATEGORY = "Network";
			return CATEGORY;
		}
	};

	const Debug DEBUG;

	Poco::Net::NetworkInterface findActiveNetworkInterface()
	{
	    auto interfaces = Poco::Net::NetworkInterface::list();

	    for( auto& interface : interfaces ) {
	        if( interface.address().isWildcard() )
	        	continue;

	        if( interface.address().isLoopback() )
	        	continue;

	        if( interface.supportsIPv4() || interface.supportsIPv6() )
	        	return interface;
	    }
	    throw Poco::IOException("No configured Ethernet interface found");
	}
}

namespace {
	std::unique_ptr<ServerSocket> createServer( Poco::UInt16 incomingPort ) {
		return std::unique_ptr<ServerSocket>( new ServerSocket( incomingPort ) );
	}
}

Network::Network( std::string uuid ) :
	_reactor(),
	uuid  (uuid)
{
	try {
		static const Poco::UInt32 START_PORT = 50123;
		size_t try_count = 100;
		for( auto port = START_PORT; true; ++port ) {
			try {
				// Try to register a server instance on a
				// particular port
				_server = createServer( port );
			} catch( const NetException& error ) {
				if( try_count == 0 )
					// Stop trying
					throw error;

				// Let's try a few more times
				--try_count;
				continue;
			}

			// Server was successfully created
			auto interface = findActiveNetworkInterface();
			break;
		}

	} catch( ... ) {
		auto eptr = std::current_exception();
		// Invoke cleanup
		Destroy();
		std::rethrow_exception( eptr );
	}

	worker = [this]() -> std::future<void> {
		// Start Networking
		auto worker = std::bind( &hack::net::Network::ExecuteWorker, std::ref(*this) );
		return std::async( ASYNC_POLICY, worker );
	}();
}

void Network::Destroy() {
}

Network::~Network() {

	std::lock_guard<std::recursive_mutex> peersLock( _peers.lock );

	for( auto& element : _peers.connected ) {
		//element.Disconnect();
	}

	SaveStopWorker();

	Destroy();
}

void Network::Setup() {
#if 0
	STUN_Request stunTask;
	RegistrationRequest registryTask;
	
	registryTask.get();
	stunTask.get();
#endif
}

void Network::SetConnectCallback(std::function<void(std::shared_ptr<hack::net::Network::Peer>)> callback) {
	_connectCallback = callback;
}

void Network::SetConnectFailedCallback(std::function<void(const std::string&, size_t)> callback) {
	_connectFailedCallback = callback;
}

void Network::SetDisconnectCallback(std::function<void(hack::net::Network::Peer&)> callback) {
	_disconnectCallback = callback;
}


Network::Address::Address( const std::string& host, Poco::UInt16 port, std::string uuid ) :
	SocketAddress( host, port ),
	uuid          ( std::move(uuid) )
{
}

Network::Address::Address( const SocketAddress& address, std::string uuid) :
	SocketAddress( address ),
	uuid         ( uuid )
{
}

Network::Address::Address( Address&& other ) :
	SocketAddress( other ),
	uuid          ( std::move(other.uuid) )
{
}

Network::Address& Network::Address::operator=( Address&& other ) {
	SocketAddress::operator=( other );
	uuid = std::move(other.uuid);
	return *this;
}

const std::string& Network::Address::GetUUID() const {
	return uuid;
}

bool
Network::Address::operator <(const Address& other) const {
	return uuid < other.uuid;
}

Network::Peer::Peer( Network& network, StreamSocket& socket, SocketReactor& reactor, std::string uuid ) :
	uuid     ( uuid ),
	_socket  ( socket ),
	_reactor ( reactor ),
	_network ( network )
{
    _reactor.addEventHandler( _socket, Poco::NObserver<Peer, ReadableNotification>(*this, &Peer::OnReadable));
    _reactor.addEventHandler( _socket, Poco::NObserver<Peer, ShutdownNotification>(*this, &Peer::OnShutdown));
}

void Network::Peer::OnReadable( const Poco::AutoPtr<ReadableNotification>& ) {
	if( _socket.available() < sizeof(Poco::UInt32)  )
		return; // Cannot yet read the size

	SocketStream stream( _socket );
	auto position = stream.tellg();
	Poco::UInt32 packetSize;
	stream >> packetSize;

	if( _socket.available() < (sizeof(Poco::UInt32) + packetSize) ) {
		// Revert what we did to the stream
		stream.seekg( position );
		return; // Cannot swallow whole package yet
	}

	std::string buffer(packetSize, '\0');
	stream.read( &buffer.front(), buffer.size() );

	// Validate size of buffer
	buffer.resize( strnlen(buffer.data(), buffer.size()) );
	receiveCallback( std::move(buffer) );
}

void Network::Peer::OnShutdown( const Poco::AutoPtr<ShutdownNotification>& ) {
	_network.OnDisconnect( _socket );
}

Network::Peer::~Peer() {

    _reactor.removeEventHandler(_socket, Poco::NObserver<Peer, ReadableNotification>(*this, &Peer::OnReadable));
    _reactor.removeEventHandler(_socket, Poco::NObserver<Peer, ShutdownNotification>(*this, &Peer::OnShutdown));
}

bool
Network::Peer::operator <(const Peer& other) const {

	if( _socket.address().host() != other._socket.address().host() )
		return _socket.address().host() < other._socket.address().host();

	if( _socket.address().port() != other._socket.address().port() )
		return _socket.address().port() < other._socket.address().port();

	return uuid < other.uuid;
}

bool Network::Peer::operator ==(const Peer& other) const {
	return uuid == other.uuid;
}

std::shared_ptr<Network::Peer> Network::CreatePeer( StreamSocket& socket, SocketReactor& reactor, std::string uuid ) {
	// We have to build Peer manually on heap here because we only allow Network to
	// instantiate Peer objects
	std::shared_ptr<Peer> sharedPeer( new Peer( *this, socket, reactor, uuid ) );

	{
		std::lock_guard<std::recursive_mutex> lock( _peers.lock );

		Address address(socket.address(), uuid);
		auto insertPair = _peers.connected.insert( std::make_pair( std::move(address), std::shared_ptr<Peer>(sharedPeer) ) );

		if( !insertPair.second )
			return insertPair.first->second; // Already present and connected!?

		address = Address(socket.address(), uuid);
		_peers.unconnected.erase( std::move(address) );
	}

	_connectCallback( sharedPeer );
	return sharedPeer;
}
#if 0
void Network::Peers::AbortWait( const StreamSocket& socket ) {
	std::lock_guard<std::recursive_mutex> lock( this->lock );
	awaitingConnection.erase( socket );
	awaitingHandshake .erase( socket );
}
#endif

StreamSocket& Network::Peer::GetSocket() {
	return _socket;
}

void Network::HandleUnconnected() {

	std::lock_guard<std::recursive_mutex> lock( _peers.lock );

	std::vector<std::shared_ptr<Peer> > peers;
	peers.reserve( _peers.unconnected.size() );

	// We need to make the manual it handling, so we properly handle
	// iterator, so we can add and remove elements within the loop.
	for( auto it = _peers.unconnected.begin(); it != _peers.unconnected.end(); ) {
		auto& entry = *it;
		auto ipAddress = entry.host().toString();

		std::string addressStr = [&]() {
			std::stringstream stream;
			stream << ipAddress << ':' << entry.port();
			return stream.str();
		}();

		DEBUG.LOG_ENTRY( "Starting Connection to " + addressStr );

		Poco::Net::StreamSocket socket( entry );
		Poco::Net::SocketStream stream(socket);

		DEBUG.LOG_ENTRY( "Sending Handshake to " + addressStr );

		stream << _createPacket( uuid );
		stream.flush(); // Make sure data is not in buffer

		DEBUG.LOG_ENTRY( "Waiting on Handshake from " + addressStr);

		std::string otherUuid(36, '\0');
		stream.read( &otherUuid.front(), otherUuid.size() );

		// We already have to advance here, so following code can modify
		// _peers.unconnected
		++it;
		peers.push_back( FinishHandshake( socket, _reactor, std::move(otherUuid) ) );

		DEBUG.LOG_ENTRY( "Received Handshake from and established Connection with " + addressStr );
	}
}

void Network::HandleUnsent() {
	typedef decltype(_queues.input) queue_type;
	queue_type inputQueue;

	{
		std::lock_guard<std::recursive_mutex> lock( _queues.lock );
		inputQueue.swap( _queues.input );
	}

	for( auto& element : inputQueue ) {

		if( element.peer ) {
			auto& socket = element.peer->GetSocket();
			DEBUG.LOG_ENTRY( std::stringstream() << "Sending packet to "
			                 << socket.address().host().toString() << ':'
			                 << socket.address().port() );
			SendPacket( socket, element.packet );
		} else
			SendPacket( element.packet );
	}
}

std::shared_ptr<Network::Peer> Network::OnConnect( StreamSocket& socket, SocketReactor& reactor ) {
	const auto peerHost = socket.address().host().toString();
	const auto peerPort = socket.address().port();

	SocketStream stream( socket );
	DEBUG.LOG_ENTRY( std::stringstream() << "Received connection and wait for Handshake from "
					 << peerHost << ':' << peerPort );

	std::lock_guard<std::recursive_mutex> lock( _peers.lock );
#if 0
	_peers.awaitingConnection.erase( socket );

	_peers.awaitingHandshake.insert( std::make_pair( socket, std::move(otherUuid) ) );
#endif

	std::string otherUuid(36, '\0');
	stream.read( &otherUuid.front(), otherUuid.size() );

	return FinishHandshake( socket, reactor, std::move(otherUuid) );
}

std::shared_ptr<Network::Peer> Network::FinishHandshake( StreamSocket& socket, SocketReactor& reactor, std::string otherUuid ) {
	const auto peerHost = socket.address().host().toString();
	const auto peerPort = socket.address().port();
	DEBUG.LOG_ENTRY( std::stringstream() << "Received handshake from "
	                 << peerHost << ':' << peerPort );

	return CreatePeer( socket, reactor, otherUuid );
}

#if 0
void Network::OnReceive() {
	const auto peerHost = GetIPAddress( *event.peer );
	const auto peerPort = event.peer->address.port;
	DEBUG.LOG_ENTRY( std::stringstream() << "Packet received." << std::endl
					  << "\tLength: " << event.packet->dataLength
					  << "\tFrom: " << peerHost << ":" << peerPort);
	{
		std::lock_guard<std::recursive_mutex> lock( _peers.lock );
		bool isFullyConnected = [&]() -> bool {
			// Check whether this is the first packet after
			// starting the connection AKA Handshake
			auto it = _peers.awaitingHandshake.begin();

			for( ; it != _peers.awaitingHandshake.end(); ++it ) {
				if( it->first->address.host == event.peer->address.host
				 && it->first->address.port == peerPort ) {
					break;
				}
			}

			if( it == _peers.awaitingHandshake.end() )
				return true;

			_peers.awaitingHandshake.erase( it );
			return false;
		}();

		if( isFullyConnected ) {
			// Handshake was already done,
			// continue with normal processing
			auto peer = extractPeer();
			peer->Receive( *event.packet );
			DEBUG.LOG_ENTRY(std::stringstream() << "Received packet from " << peerHost << ':' << peerPort);
		} else {
			// Received Handshake

			// Peer sent UUID
			CreatePeer( *event.peer, Peers::ExtractUUID( event.packet ) );
			DEBUG.LOG_ENTRY(std::stringstream() << "Received handshake from " << peerHost << ':' << peerPort);
		}
	}

	// Clean up the packet now that we're done using it.
	enet_packet_destroy(event.packet);
}
#endif

void Network::OnDisconnect( StreamSocket& socket ) {
	auto peerHost = socket.address().host();
	auto peerHostStr = peerHost.toString();
	auto peerPort = socket.address().port();

	DEBUG.LOG_ENTRY(std::stringstream() << "Disconnected: " << peerHostStr << ':' << peerPort);

	decltype(_peers.connected)::iterator it;

	{
		// Limit lock because later on we invoke callbacks and do not
		// know how long they block
		std::lock_guard<std::recursive_mutex> lock( _peers.lock );

		typedef decltype(_peers.connected) connected_type;
		auto isSame = [&]( const connected_type::value_type& element ) -> bool {
			return element.first.host() == peerHost && element.first.port() == peerPort;
		};

		it = std::find_if( _peers.connected.begin(),
		                   _peers.connected.end(),
		                   isSame );
#if 0
		if( it == _peers.connected.end() ) {
			// Skip waiting for handshake
			_peers.AbortWait( socket );
		} else {
			// The user data of Event is already set to null so
			// we are allowed to remove the shared_ptr
#endif
			_peers.connected.erase( it );
#if 0
		}
#endif
	}

	if( it == _peers.connected.end() )
		_connectFailedCallback( peerHostStr, peerPort );
	else
		_disconnectCallback( *(it->second) );
}

void Network::ExecuteWorker() {

	DEBUG.LOG_ENTRY("[Worker] Start...");

	// Used to satisfy Poco interface needs
	struct PeerWrapper {
		std::shared_ptr<Peer> peer;
		PeerWrapper(Poco::Net::StreamSocket&, Poco::Net::SocketReactor&) {}
	};

	struct Acceptor : public SocketAcceptor<PeerWrapper> {
		Network& network;
		Acceptor( Network& network, ServerSocket& socket, SocketReactor& reactor ) :
			SocketAcceptor<PeerWrapper>(socket, reactor),
			network(network)
		{
		}

		PeerWrapper* createServiceHandler( StreamSocket& socket ) override {
			auto wrapper = new PeerWrapper( socket, *reactor() );
			wrapper->peer = network.OnConnect( socket, *reactor() );
			return wrapper;
		}
	};

	// Accept incoming packets
	Acceptor acceptor( *this, *_server, _reactor );
	_reactor.setTimeout( Poco::Timespan(0, 2000) );
	_reactor.addEventHandler( *_server, Poco::NObserver<Network, Poco::Net::TimeoutNotification>(*this, &Network::OnTimeout));

    // Run till reactor is stopped
    _reactor.run();

	DEBUG.LOG_ENTRY("[Worker] ...Stop");
}

void Network::OnTimeout( const Poco::AutoPtr<TimeoutNotification>& ) {
	HandleUnconnected();
	HandleUnsent();
}

void Network::StopWorker() {
	DEBUG.LOG_ENTRY("[Worker] Stopping...");
	_reactor.stop();
}

bool Network::ConnectTo( const std::string& host, Poco::UInt32 port, std::string uuid ) {

	if( host == GetIPAddress() && port == GetIncomingPort() )
		return false; // No sense in connecting to ourselves

	// Make sure we are the only thread accessing object
	std::lock_guard<std::recursive_mutex> lock( _peers.lock );
	_peers.unconnected.insert( Address( host, port, uuid ) );
	return true;
}

Poco::UInt32 Network::GetIncomingPort() const {
	return _server->address().port();
}

std::string Network::GetIPAddress() const {
	return _server->address().host().toString();
}

void Network::SendPacket( StreamSocket& socket, const packet_type& packet ) {
	SocketStream stream( socket );
	stream << packet;
	stream.flush();
}

void Network::SendPacket( const packet_type& packet ) {
	std::lock_guard<std::recursive_mutex> lock( _peers.lock );

	for( auto& peer : _peers.connected ) {
		auto& peerSocket = peer.second->GetSocket();
		SocketStream stream( peerSocket );
		stream << packet;
		stream.flush();
	}
}

void Network::Enqueue( std::shared_ptr<Peer> peer, packet_type packet ) {
	queue_element_type element;
	element.packet = packet;
	element.peer = std::move(peer);
	std::lock_guard<std::recursive_mutex> lock( _queues.lock );
	_queues.input.push_back( std::move(element) );
}
