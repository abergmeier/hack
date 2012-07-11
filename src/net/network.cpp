
#include <algorithm>
#include <vector>
#include <string>
//#include <Poco/Net/MulticastSocket.h>
#include <chrono>
#include <exception>
#include <iostream>
#include <sstream>
#include <tuple>
#include <string.h> //for strnlen
#include <Poco/Net/NetworkInterface.h>
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

bool operator<(const ENetAddress& lhs, const ENetAddress& rhs) {
	if( lhs.host == rhs.host )
		return lhs.port < rhs.port;

	return lhs.host < rhs.host;
}

namespace {
	ENetHost* createServer( enet_uint16 incomingPort ) {
		// Create Server
		ENetAddress address;

	    address.host = ENET_HOST_ANY;
	    address.port = incomingPort;
		auto server = enet_host_create( &address, // create a server host
		                                32, // allowed 32 clients
					                    2, // allow up 2 channels to be used, 0 and 1
					                    0, // any downstream bandwidth
					                    0  // any upstream bandwidth
		);

		if( server == nullptr )
			throw std::runtime_error("An error occurred while trying to create an ENet server.");

		return server;
	}
}

Network::Network( std::string uuid ) :
	_state(STOPPED),
	uuid  (uuid)
{
	_queues[0] = std::unique_ptr<std::deque<queue_element_type>>(new std::deque<queue_element_type>());
	_queues[1] = std::unique_ptr<std::deque<queue_element_type>>(new std::deque<queue_element_type>());

	_atomicQueues[0] = _queues[0].get();
	_atomicQueues[1] = _queues[1].get();

	if( enet_initialize() != 0 )
		throw std::runtime_error("Could not initialize ENET.");

	try {
		static const enet_uint16 START_PORT = 50123;
		size_t try_count = 100;
		for( auto port = START_PORT; true; ++port ) {
			try {
				// Try to register a server instance on a
				// particular port
				_server = createServer( port );
			} catch( const std::runtime_error& error ) {
				if( try_count == 0 )
					// Stop trying
					throw error;

				// Let's try a few more times
				--try_count;
				continue;
			}

			// Server was successfully created
			auto interface = findActiveNetworkInterface();
			_host = interface.address().toString();
			_port = port;
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
	_server = nullptr;
	enet_deinitialize();
}

Network::~Network() {

	std::lock_guard<std::recursive_mutex> peersLock( _peers.lock );

	// First wait on all timeouts
	for( auto& timeout : _peers.connectionTimeout ) {
		timeout.second.wait();
	}

	_peers.connectionTimeout.clear();
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

#if 0
void Network::Receive() {
	Poco::Net::SocketAddress address("other", 514);
	Poco::Net::MulticastSocket socket(Poco::Net::IPAddress(), address.port());

	// to receive any data you must join
	socket.joinGroup(address.host());

	char buffer[1024];

	for (;;)
	{
		Poco::Net::SocketAddress sender;
		int n = socket.receiveFrom(buffer, sizeof(buffer)-1, sender);
		buffer[n] = '\0';
		std::cout << sender.toString() << ": " << buffer << std::endl;
	}
}

#endif

Network::Address::Address( const std::string& host, enet_uint16 port, std::string uuid ) :
	ENetAddress(),
	uuid(uuid)
{
	this->port = port;

	// Connect to other peer
	if( enet_address_set_host( this, host.c_str() ) != 0 ) {
		throw std::runtime_error("Could no set address");
	}
}

Network::Address::Address( ENetAddress address, std::string uuid ) :
	ENetAddress( std::forward<ENetAddress>(address) ),
	uuid( std::forward<std::string>(uuid) )
{
}

Network::Address::Address( Address&& other ) :
	ENetAddress( other ),
	uuid( std::move(other.uuid) )
{
}

bool
Network::Address::operator <(const Address& other) const {
	return uuid == other.uuid;
}

Network::Peer::Peer( ENetPeer& peer, std::string uuid ) :
	uuid    ( uuid ),
	address ( peer.address ),
	enetPeer( &peer )
	ipAddress( GetIPAddress( peer ) ),
{
}

Network::Peer::~Peer() {

	if( enetPeer == nullptr )
		return; //already cleaned up

	enet_peer_reset( enetPeer );
}

bool
Network::Peer::operator <(const Peer& other) const {
	if( address.host != other.address.host )
		return address.host < other.address.host;

	if( address.port != other.address.port )
		return address.port < other.address.port;

	return uuid < other.uuid;
}

bool Network::Peer::operator ==(const Peer& other) const {
	return uuid == other.uuid;
}

void
Network::Peer::Disconnect() {
	enet_peer_disconnect( enetPeer, 0 );
}

void
Network::Peer::Receive(const ENetPacket& packet) {
	buffer_type data;
	data.reserve(packet.dataLength);
	data.assign(packet.data, packet.data + packet.dataLength);
	receiveCallback(data);
}
#if 0
const std::string&
Network::Peer::GetUUID() const {
	return _uuid;
}

#endif

bool
Network::IsConnecting( const std::string& uuid ) const {
	std::lock_guard<std::recursive_mutex> lock( _peers.lock );

	for( const auto& element : _peers.unconnected ) {
		if( element.uuid == uuid )
			return true;
	}

	for( const auto& element : _peers.awaitingConnection ) {
		if( element.second == uuid )
			return true;
	}

	for( const auto& element : _peers.awaitingHandshake ) {
		if( element.second == uuid )
			return true;
	}

	return false;
}

bool
Network::IsConnected( const std::string& uuid) const {
	std::lock_guard<std::recursive_mutex> lock( _peers.lock );

	for( const auto& element : _peers.connected ) {
		if( element.first.uuid == uuid )
			return true;
	}

	return false;
}

bool
Network::WaitUntilConnected( const std::string& uuid ) const {
	static const std::chrono::milliseconds SLEEP_MS( 5 );

	// Wait for outstanding connection phase
	while( IsConnecting( uuid ) )
		std::this_thread::sleep_for( SLEEP_MS );

	return IsConnected( uuid );
}

void Network::CreatePeer( ENetPeer& peer, std::string uuid ) {
	// We have to build Peer manually on heap here because we only allow Network to
	// instantiate Peer objects
	auto sharedPeer = std::shared_ptr<Peer>( new Peer( peer, uuid ) );

	{
		std::lock_guard<std::recursive_mutex> lock( _peers.lock );
		auto insertPair = _peers.connected.insert( std::make_pair( Address(sharedPeer->address, uuid), sharedPeer ) );

		if( !insertPair.second )
			return; // Already present and connected!?

		_peers.unconnected.erase( Address(sharedPeer->address, uuid) );
		peer.data = &_peers.connected.at( Address(sharedPeer->address, uuid) );
		//peer.data = &(*insertPair.first);
	}

	_connectCallback( sharedPeer );
}

std::string Network::Peers::ExtractUUID( ENetPacket* packet ) {
	// Assume we got a UUID sent
	char* uuidPtr = reinterpret_cast<char*>( packet->data );
	// Make sure we do not access invalid memory
	// when data is corrupted
	const auto uuidLen = strnlen( uuidPtr, packet->dataLength );
	return std::string( uuidPtr, uuidLen );
}

void Network::Peers::AbortWait( ENetPeer& peer ) {
	std::lock_guard<std::recursive_mutex> lock( this->lock );
	awaitingConnection.erase( &peer );
	awaitingHandshake .erase( &peer );

	auto timeoutIt = connectionTimeout.find( &peer );

	if( timeoutIt != connectionTimeout.end() ) {
		timeoutIt->second.wait();
		connectionTimeout.erase( timeoutIt );
	}

	// We could not set up a connection before
	// (for whatever reason). Clean up peer.
	enet_peer_reset( &peer );
}

void Network::HandleUnconnected() {

	//
	// Do connection handling
	//
	//typedef decltype(_peers.unconnected) connections_type;

	std::lock_guard<std::recursive_mutex> lock( _peers.lock );

	for( auto& entry : _peers.unconnected ) {

		DEBUG.LOG_ENTRY(std::stringstream() << "Connecting to " << entry.host << ':' << entry.port);
		// Initiate the connection, allocating the two channels 0 and 1. */
		auto peer = enet_host_connect( _server, &entry, 2, 0);

		if (peer == nullptr) {
		   DEBUG.ERR_ENTRY(std::stringstream() << "Setting up connection !FAILED!");
		   exit (-1);
		}

		enet_peer_timeout( peer,
				           100, //timeoutLimit
                           0, //timeoutMinimum
                           0  //timeoutMaximum
		);

		// Check whether it is already in
		if( _peers.awaitingHandshake.find( peer ) != _peers.awaitingHandshake.end()
		 || _peers.connected.find( Address(peer->address, entry.uuid) ) != _peers.connected.end() ) {
			// Found in later stage
			continue;
		}

#if 0
		static const auto ASYNC_POLICY = std::launch::async;

		// Setup connection timeout
		auto timeoutHandler = [peer, this]() {
			static const std::chrono::milliseconds TIMEOUT_MS( 2000 );

			std::this_thread::sleep_for( TIMEOUT_MS );

			auto& peerObject = *peer;

			std::lock_guard<std::recursive_mutex> lock( _peers.lock );
			_peers.AbortWait( peerObject );
		};

		_peers.connectionTimeout.insert( std::make_pair( peer, std::async( ASYNC_POLICY, timeoutHandler ) ) );
#endif
		_peers.awaitingConnection.insert( std::make_pair(peer, entry.uuid) );
		DEBUG.ERR_ENTRY(std::stringstream() << "Trying to establish connection with " << peer->address.host << ':' << peer->address.port);
	}

	_peers.unconnected.clear();
}

bool Network::_ExecuteWorker() {

	ENetEvent event;

	HandleUnconnected();

	//
	// Do input handling
	//

	auto extractPeer = [&event]() -> std::shared_ptr<Peer> {
		// We save an address to shared_ptr inside event
		// Cast it to its original format
		auto sharedPtr = static_cast<std::shared_ptr<Peer>*>(event.peer->data);
		// Using shared_ptr is safest when copying - do that
		auto copyOfSharedPtr = *sharedPtr;
		// Pass out our copy
		return std::move(copyOfSharedPtr);
	};

	// Wait up to 5 milliseconds for an event.
	if( enet_host_service( _server, &event, 5 ) > 0 ) {
		switch( event.type ) {
			case ENET_EVENT_TYPE_NONE:
				break;
			case ENET_EVENT_TYPE_CONNECT: {
				const auto peerHost = GetIPAddress( *event.peer );
				const auto peerPort = event.peer->address.port;
				DEBUG.LOG_ENTRY( std::stringstream() << "Peer connected from "
				                 << peerHost << ':'
				                 << peerPort);

				SendTo( event.peer, uuid );
				DEBUG.LOG_ENTRY(std::stringstream() << "Waiting for UUID handshake from " << peerHost << ':' << peerPort);

				{
					std::lock_guard<std::recursive_mutex> lock( _peers.lock );
					auto it = _peers.awaitingConnection.find( event.peer );

					// When we await connection, we initiated it
					if( it == _peers.awaitingConnection.end() ) {
						// Play passive role in connection establishment
						// First wait on other UUID
						_peers.awaitingHandshake.insert( std::make_pair( event.peer, std::string() ) );
					} else {
						// Play active role in connection establishment
						// Send our UUID to Peer

						std::string otherUuid = (*it).second;
						_peers.awaitingHandshake.insert( std::make_pair( event.peer, std::move(otherUuid) ) );
					}

					if( it != _peers.awaitingConnection.end() )
						_peers.awaitingConnection.erase( it );
				}

				break;
			}
			case ENET_EVENT_TYPE_RECEIVE: {
				const auto peerHost = GetIPAddress( *event.peer );
				const auto peerPort = event.peer->address.port;
				DEBUG.LOG_ENTRY( std::stringstream() << "Packet received." << std::endl
				                  << "\tLength: " << event.packet->dataLength
				                  << "\tFrom: " << peerHost << ":" << peerPort);

				{
					std::lock_guard<std::recursive_mutex> lock( _peers.lock );

					bool isConnected = false;

					for( const auto& address : _peers.connected ) {
						if( address.first.host == peerHost
						 && address.first.port == peerPort ) {
							isConnected = true;
							break;
						}
					}

					if( isConnected ) {
						// Handshake was already done
						// with normal processing
						auto peer = extractPeer();
						peer->Receive( *event.packet );
					} else {
						// Received Handshake

						// Peer sent UUID
						CreatePeer( *event.peer, Peers::ExtractUUID( event.packet ) );
						DEBUG.LOG_ENTRY(std::stringstream() << "Waiting for UUID handshake from " << peerHost << ':' << peerPort << "!SUCCEEDED!");

						_peers.AbortWait( *event.peer );
					}
				}

				// Clean up the packet now that we're done using it.
				enet_packet_destroy(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT: {
				auto peer = event.peer;
				auto host = GetIPAddress( *event.peer );
				auto port = event.peer->address.port;
				std::cout << "A client disconnected: " << peer->address.host
						  << ":" << peer->address.port
						  << std::endl;

				{
					std::lock_guard<std::recursive_mutex> lock( _peers.lock );

					typedef decltype(_peers.connected) connected_type;
					auto isSame = [&host, &port]( const connected_type::value_type& element ) -> bool {
						return element.first.host == host && element.first.port == port;
					};

					auto it = std::find_if( _peers.connected.begin(),
					                        _peers.connected.end(),
					                        isSame );

					if( it != _peers.connected.end() ) {
						// We set the user data of Event to null so
						// we are allowed to remove the shared_ptr
						_disconnectCallback( *(it->second) );
						_peers.connected.erase ( it );
					}

					// Skip waiting for handshake
					_peers.AbortWait( *event.peer );
				}
				break;
			}
		}
	}

	//
	// Do output handling
	//

	// Exchange public with private queue so other
	// threads may fill the external queue without
	// need to do complicated locking
	_atomicQueues[0].exchange(_atomicQueues[1]);

	// We now can process the new private queue
	// since it is no longer used by another thread

	//auto& currentQueue = *_atomicQueues[1].load();
	//TODO: introduce locking
	// Process everything we have in the queue
#if 0
	for( auto& element : currentQueue ) {
		if( element.peer )
			// Send to peer
			element.peer->Send( element.buffer, element.callback );
		else
			// Do broadcast
			Send( element.buffer, element.callback );
	}
#endif

	return _state == RUNNING;
}

void Network::ExecuteWorker() {
	static const std::chrono::milliseconds DURATION( 1 );

	_state = RUNNING;

	DEBUG.LOG_ENTRY("[Worker] Start...");

	while( _ExecuteWorker() ) {

		auto queue = _atomicQueues[0].load();
		if( queue == nullptr )
			// Should never happen but paranoia is so sweet
			continue;

		if( queue->empty() )
			// If there is nothing to do - do not spam
			// the CPU
			std::this_thread::sleep_for( DURATION );
	}

	_state = STOPPED;

	DEBUG.LOG_ENTRY("[Worker] ...Stop");
}

void Network::StopWorker() {
	DEBUG.LOG_ENTRY("[Worker] Stopping...");
	_state = HALTING;
}

void Network::ConnectTo( const std::string& host, enet_uint16 port, std::string uuid ) {

	if( host == GetIPAddress() && port == GetIncomingPort() )
		return; // No sense in connecting to ourselves

	// Make sure we are the only thread accessing object
	std::lock_guard<std::recursive_mutex> lock( _peers.lock );
	_peers.unconnected.insert( Address( host, port, uuid ) );
}

enet_uint16 Network::GetIncomingPort() const {
	return _port;
}

const std::string& Network::GetIPAddress() const {
	return _host;
}

std::string Network::GetIPAddress( const ENetPeer& peer ) {

	std::string hostName(100, '\0');
	if( enet_address_get_host_ip( &peer.address,
								  &hostName.front(), hostName.length() ) < 0 )
		hostName.clear();
	else {
		// Validate size of string
		hostName.resize( strnlen(hostName.data(), hostName.length()) );
	}
	return hostName;
}
