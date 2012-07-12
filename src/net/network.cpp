
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
		                                0, // allow as many simultaneous connections as possible
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

	for( auto& element : _peers.connected ) {
		enet_peer_disconnect_later( element.second->enetPeer, 0 );
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

Network::Address::Address( const std::string& host, enet_uint16 port, std::string uuid ) :
	ENetAddress(),
	uuid(uuid),
	ipAddress(host)
{
	this->port = port;

	// Connect to other peer
	if( enet_address_set_host( this, host.c_str() ) != 0 ) {
		throw std::runtime_error("Could no set address");
	}
}

Network::Address::Address( Address&& other ) :
	ENetAddress( std::forward<ENetAddress>(other) ),
	uuid       ( std::move(other.uuid) ),
	ipAddress  ( std::move(other.ipAddress) )
{
}

bool
Network::Address::operator <(const Address& other) const {
	return uuid < other.uuid;
}

Network::Peer::Peer( ENetPeer& peer, std::string uuid ) :
	uuid     ( uuid ),
	address  ( peer.address ),
	ipAddress( GetIPAddress( peer ) ),
	enetPeer ( &peer )
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

void Network::CreatePeer( ENetPeer& peer, std::string uuid ) {
	// We have to build Peer manually on heap here because we only allow Network to
	// instantiate Peer objects
	std::shared_ptr<Peer> sharedPeer( new Peer( peer, uuid ) );

	{
		std::lock_guard<std::recursive_mutex> lock( _peers.lock );
		const auto ipAddress = GetIPAddress( *sharedPeer->enetPeer );
		auto insertPair = _peers.connected.insert( std::make_pair( Address(ipAddress, sharedPeer->address.port, uuid), std::shared_ptr<Peer>(sharedPeer) ) );

		if( !insertPair.second )
			return; // Already present and connected!?

		Address address(ipAddress, sharedPeer->address.port, uuid);
		_peers.unconnected.erase( address );
		_peers.AbortWait( peer );

		// Make sure we have a pointer to our Peer Wrapper
		std::shared_ptr<Peer>* peerPtr = &insertPair.first->second;
		peer.data = peerPtr;
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

		DEBUG.LOG_ENTRY(std::stringstream() << "Connecting to " << entry.ipAddress << ':' << entry.port);
		// Initiate the connection, allocating the two channels 0 and 1. */
		auto peer = enet_host_connect( _server, &entry, 2, 0);

		if (peer == nullptr) {
		   DEBUG.ERR_ENTRY(std::stringstream() << "Setting up connection !FAILED!");
		   exit (-1);
		}

		// Check whether it is already in
		auto ipAddress = GetIPAddress( *peer );
		if( _peers.awaitingHandshake.find( peer ) != _peers.awaitingHandshake.end()
		 || _peers.connected.find( Address(ipAddress, peer->address.port, entry.uuid) ) != _peers.connected.end() ) {
			// Found in later stage
			continue;
		}

		_peers.awaitingConnection.insert( std::make_pair(peer, entry.uuid) );
		DEBUG.ERR_ENTRY(std::stringstream() << "Trying to establish connection with " << ipAddress << ':' << peer->address.port);
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
		if( element.peer )
			SendPacket( *element.peer->enetPeer, element.packet );
		else
			SendPacket( *_server, element.packet );
	}
}

bool Network::_ExecuteWorker() {

	HandleUnconnected();

	HandleUnsent();

	ENetEvent event;

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
				                 << peerHost << ':' << peerPort);

				_SendTo( *event.peer, uuid );
				DEBUG.LOG_ENTRY( std::stringstream() << "Sent Handshake to "
				                 << peerHost << ':' << peerPort);

				std::lock_guard<std::recursive_mutex> lock( _peers.lock );
				auto it = _peers.awaitingConnection.find( event.peer );

				std::string otherUuid;

				// Add known UUID information
				if( it != _peers.awaitingConnection.end() ) {
					otherUuid = (*it).second;
					_peers.awaitingConnection.erase( it );
				}

				_peers.awaitingHandshake.insert( std::make_pair( event.peer, std::move(otherUuid) ) );

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
					} else {
						// Received Handshake

						// Peer sent UUID
						CreatePeer( *event.peer, Peers::ExtractUUID( event.packet ) );
						DEBUG.LOG_ENTRY(std::stringstream() << "Received handshake from " << peerHost << ':' << peerPort);
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

				DEBUG.LOG_ENTRY(std::stringstream() << "Disconnected: " << host << ':' << peer->address.port);

				{
					std::lock_guard<std::recursive_mutex> lock( _peers.lock );

					typedef decltype(_peers.connected) connected_type;
					auto isSame = [&]( const connected_type::value_type& element ) -> bool {
						return element.first.host == event.peer->address.host && element.first.port == port;
					};

					auto it = std::find_if( _peers.connected.begin(),
					                        _peers.connected.end(),
					                        isSame );

					if( it == _peers.connected.end() ) {
						// Skip waiting for handshake
						_peers.AbortWait( *event.peer );
						// Handshake failed
						_connectFailedCallback( host, port );
					} else {
						// The user data of Event is already set to null so
						// we are allowed to remove the shared_ptr
						_disconnectCallback( *(it->second) );
						_peers.connected.erase ( it );
					}
				}
				break;
			}
		}
	}

	return _state == RUNNING;
}

void Network::ExecuteWorker() {
	_state = RUNNING;

	DEBUG.LOG_ENTRY("[Worker] Start...");

	while( _ExecuteWorker() ) {
		// We do not sleep here because we
		// internally already use enet's query
		// mechanism
	}

	_state = STOPPED;

	DEBUG.LOG_ENTRY("[Worker] ...Stop");
}

void Network::StopWorker() {
	DEBUG.LOG_ENTRY("[Worker] Stopping...");
	_state = HALTING;
}

bool Network::ConnectTo( const std::string& host, enet_uint16 port, std::string uuid ) {

	if( host == GetIPAddress() && port == GetIncomingPort() )
		return false; // No sense in connecting to ourselves

	// Make sure we are the only thread accessing object
	std::lock_guard<std::recursive_mutex> lock( _peers.lock );
	_peers.unconnected.insert( Address( host, port, uuid ) );
	return true;
}

enet_uint16 Network::GetIncomingPort() const {
	return _port;
}

const std::string& Network::GetIPAddress() const {
	return _host;
}

std::string Network::GetIPAddress( const ENetPeer& peer ) {

	// Normally 128 should be sufficient even when representing
	// IPv6 in binary (+7 delimiter)
	std::string ip(128 + 7, '\0');
	if( enet_address_get_host_ip( &peer.address,
								  &ip.front(), ip.length() ) < 0 )
		ip.clear();
	else {
		// Validate size of string
		ip.resize( strnlen(ip.data(), ip.length()) );
	}
	return ip;
}

void Network::SendPacket( ENetPeer& peer, ENetPacket* packet ) {
	if( enet_peer_send( &peer, 0, packet ) != 0 )
		throw std::runtime_error("SEND FAIL");
	// enet_peer_send handles enet_packet_destroy()

	enet_host_flush( peer.host );
}

void Network::SendPacket( ENetHost& host, ENetPacket* packet ) {
	enet_host_broadcast( &host, 0, packet );
	// enet_peer_send handles enet_packet_destroy()

	enet_host_flush( &host );
}

void Network::Enqueue( std::shared_ptr<Peer> peer, ENetPacket* packet ) {
	queue_element_type element;
	element.packet = packet;
	element.peer = std::move(peer);
	std::lock_guard<std::recursive_mutex> lock( _queues.lock );
	_queues.input.push_back( std::move(element) );
}
