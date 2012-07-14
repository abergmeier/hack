
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

	std::unique_ptr<ServerSocket> createServer( Poco::UInt16 incomingPort ) {
		return std::unique_ptr<ServerSocket>( new ServerSocket( incomingPort ) );
	}

	std::string GetAddressString( const SocketAddress& address ) {
		std::stringstream stream;
		stream << address.host().toString() << ':' << address.port();
		return stream.str();
	};
}

void Network::PeerWrapper::OnReadable( const Poco::AutoPtr<ReadableNotification>& ) {
	SocketStream stream( _socket );
	auto position = stream.tellg();

	Poco::UInt32 packetSize;
	stream >> packetSize;

	// Skip seperator
	stream.get();

	// Use std::string directly as buffer
	std::string buffer(packetSize, '\0');
	stream.read( &buffer.front(), buffer.length() );

	if( !stream.good() ) {
		stream.seekg( position );
		return;
	}

	// Validate length of buffer - prevents accidental
	// access to unallocated memory
	buffer.resize( strnlen(buffer.data(), buffer.length()) );

	std::lock_guard<std::recursive_mutex> lock( _network._peers.lock );
	auto removeCount = _network._peers.awaitingHandshake.erase( _socket );

	if( removeCount == 0 ) {
		// Handshake already successfull
		_peer->receiveCallback( std::move(buffer) );
	} else {
		_peer = _network.FinishHandshake( _socket, _reactor, std::move(buffer) );
	}
}

void Network::PeerWrapper::OnWriteable( const Poco::AutoPtr<WritableNotification>& ) {
	_network.HandleUnsent();
}

void Network::PeerWrapper::OnTimeout( const Poco::AutoPtr<TimeoutNotification>& ) {
	std::lock_guard<std::recursive_mutex> lock( _network._peers.lock );

	if( _peer ) {
		DEBUG.ERR_ENTRY( std::stringstream() << "Connection timed out " << _peer->uuid );
		_network._peers.connected.erase( Address(_peer->GetSocket().address(), _peer->uuid) );
	} else {
		DEBUG.ERR_ENTRY( std::stringstream() << "Connection timed out "
		                 << GetAddressString( _socket.address()) );
	}

	_network._peers.awaitingHandshake.erase( _socket );
}

void Network::PeerWrapper::OnError( const Poco::AutoPtr<ErrorNotification>& ) {
	DEBUG.ERR_ENTRY( std::stringstream() << "Error with "
	                 << GetAddressString( _socket.address() ) );
}

void Network::PeerWrapper::OnShutdown( const Poco::AutoPtr<ShutdownNotification>& ) {
	_network.OnDisconnect( _socket );
	delete this;
}

Network::PeerWrapper::PeerWrapper(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor) :
	_reactor(*static_cast<SocketReactor*>(nullptr)),
	_network(*static_cast<Network*>(nullptr))
{
	throw std::runtime_error("Wrong constructor");
}

Network::PeerWrapper::PeerWrapper(Network& network, Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor) :
	_peer(),
	_socket(socket),
	_reactor(reactor),
	_network(network)
{
	_reactor.addEventHandler( _socket, Poco::NObserver<PeerWrapper, ReadableNotification>(*this, &PeerWrapper::OnReadable ));
	_reactor.addEventHandler( _socket, Poco::NObserver<PeerWrapper, WritableNotification>(*this, &PeerWrapper::OnWriteable));
	_reactor.addEventHandler( _socket, Poco::NObserver<PeerWrapper, ShutdownNotification>(*this, &PeerWrapper::OnShutdown ));
	_reactor.addEventHandler( _socket, Poco::NObserver<PeerWrapper, TimeoutNotification >(*this, &PeerWrapper::OnTimeout  ));
	_reactor.addEventHandler( _socket, Poco::NObserver<PeerWrapper, ErrorNotification   >(*this, &PeerWrapper::OnError    ));
}

Network::PeerWrapper::~PeerWrapper() {
	_reactor.removeEventHandler( _socket, Poco::NObserver<PeerWrapper, ReadableNotification>(*this, &PeerWrapper::OnReadable ));
	_reactor.removeEventHandler( _socket, Poco::NObserver<PeerWrapper, WritableNotification>(*this, &PeerWrapper::OnWriteable));
	_reactor.removeEventHandler( _socket, Poco::NObserver<PeerWrapper, ShutdownNotification>(*this, &PeerWrapper::OnShutdown ));
	_reactor.removeEventHandler( _socket, Poco::NObserver<PeerWrapper, TimeoutNotification >(*this, &PeerWrapper::OnTimeout  ));
	_reactor.removeEventHandler( _socket, Poco::NObserver<PeerWrapper, ErrorNotification   >(*this, &PeerWrapper::OnError    ));
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

			break;
		}

	} catch( ... ) {
		auto eptr = std::current_exception();
		// Invoke cleanup
		std::rethrow_exception( eptr );
	}

	// Server was successfully created
	auto interface = findActiveNetworkInterface();
	_ipAddress = interface.address().toString();

	_acceptor = std::unique_ptr<Acceptor>( new Acceptor(*this, *_server, _reactor) );

	worker = [this]() -> std::future<void> {
		// Start Networking
		auto worker = std::bind( &hack::net::Network::ExecuteWorker, std::ref(*this) );
		return std::async( ASYNC_POLICY, worker );
	}();
}

Network::~Network() {

	std::lock_guard<std::recursive_mutex> peersLock( _peers.lock );

	for( auto& element : _peers.connected ) {
		//element.Disconnect();
	}

	_peers.connected.clear();

	SaveStopWorker();
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
}

Network::Peer::~Peer() {
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

		Address address( socket.address(), uuid );
		auto insertPair = _peers.connected.insert( std::make_pair( std::move(address), std::shared_ptr<Peer>(sharedPeer) ) );

		if( !insertPair.second )
			return insertPair.first->second; // Already present and connected!?

		address = Address( socket.address(), uuid );
		_peers.unconnected.erase( address );
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

void Network::ConnectOutstanding() {

	std::lock_guard<std::recursive_mutex> lock( _peers.lock );

	// We need to make the manual it handling, so we properly handle
	// iterator, so we can add and remove elements within the loop.
	for( auto& entry : _peers.unconnected ) {
		//auto ipAddress = entry.host().toString();

		const auto addressStr = GetAddressString( entry );
		DEBUG.LOG_ENTRY( std::string("Starting Connection to ") + addressStr );

		std::unique_ptr<Poco::Net::StreamSocket> socket;
		try {
			const SocketAddress& addr = entry;
			socket.reset( new Poco::Net::StreamSocket( addr ) );
		// Ignore following exception
		} catch( const InvalidAddressException&    ) {
		} catch( const HostNotFoundException&      ) {
		} catch( const NoAddressFoundException&    ) {
		// Log connection errors
		} catch( const ConnectionAbortedException& ) {
			DEBUG.ERR_ENTRY( std::string("Connection aborted to ") + addressStr );
		} catch( const ConnectionResetException&   ) {
			DEBUG.ERR_ENTRY( std::string("Connection reset to ") + addressStr);
		} catch( const ConnectionRefusedException& ) {
			DEBUG.ERR_ENTRY( std::string("Connection refused to ") + addressStr );
		} catch( const Poco::Net::NetException& ex ) {
			DEBUG.ERR_ENTRY( std::string("Connection failed to ") + addressStr + ": " + ex.displayText() );
			break;
		}

		// Register Handshake wait so we handle
		// the first packet
		_peers.awaitingHandshake.insert( std::make_pair( *socket, entry.uuid ) );

		DEBUG.LOG_ENTRY( std::string("Sending Handshake to ") + addressStr );

		auto packet = _createPacket( uuid );
		SendPacket( *socket, packet );

		DEBUG.LOG_ENTRY( std::string("Waiting on Handshake from ") + addressStr);

		_acceptor->createServiceHandler( *socket );
	}

	_peers.unconnected.clear();
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

void Network::OnConnect( StreamSocket& socket, SocketReactor& reactor ) {
	const auto peerHost = socket.address().host().toString();
	const auto peerPort = socket.address().port();

	SocketStream stream( socket );
	DEBUG.LOG_ENTRY( std::stringstream() << "Received connection and wait for Handshake from "
					 << peerHost << ':' << peerPort );

	std::lock_guard<std::recursive_mutex> lock( _peers.lock );
#if 0
	_peers.awaitingConnection.erase( socket );
#endif
	_peers.awaitingHandshake.insert( std::make_pair( socket, std::string() ) );
}

std::shared_ptr<Network::Peer> Network::FinishHandshake( StreamSocket& socket, SocketReactor& reactor, std::string otherUuid ) {
	const auto peerHost = socket.address().host().toString();
	const auto peerPort = socket.address().port();
	const auto address = GetAddressString( socket.address() );
	DEBUG.LOG_ENTRY( "Received Handshake from and established Connection with " + address );

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
	typedef decltype(_peers.connected) connected_container;
	connected_container::iterator it;

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

	_reactor.setTimeout( Poco::Timespan(0, 2000) );
	_reactor.addEventHandler( *_server, Poco::NObserver<Network, Poco::Net::TimeoutNotification>(*this, &Network::OnTimeout));

    // Run till reactor is stopped
    _reactor.run();

	DEBUG.LOG_ENTRY("[Worker] ...Stop");
}

void Network::OnTimeout( const Poco::AutoPtr<TimeoutNotification>& ) {
	ConnectOutstanding();
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
	_peers.unconnected.insert( Address( host, port, std::move(uuid) ) );
	return true;
}

Poco::UInt32 Network::GetIncomingPort() const {
	return _server->address().port();
}

std::string Network::GetIPAddress() const {
	return _ipAddress;
}

void Network::SendPacket( StreamSocket& socket, const packet_type& packet ) {
	socket.sendBytes( packet.data(), packet.length() * sizeof(packet_type::value_type) );
}

void Network::SendPacket( const packet_type& packet ) {
	std::lock_guard<std::recursive_mutex> lock( _peers.lock );

	for( auto& peer : _peers.connected ) {
		SendPacket( peer.second->GetSocket(), packet );
	}
}

void Network::Enqueue( std::shared_ptr<Peer> peer, packet_type packet ) {
	queue_element_type element;
	element.packet = packet;
	element.peer = std::move(peer);
	std::lock_guard<std::recursive_mutex> lock( _queues.lock );
	_queues.input.push_back( std::move(element) );
}
