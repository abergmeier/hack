
#include <vector>
#include <string>
//#include <Poco/Net/MulticastSocket.h>
#include <chrono>
#include <exception>
#include <iostream>
#include <sstream>

#include "network.hpp"

using namespace hack::net;

const Network::Debug Network::DEBUG;

namespace {
	bool DEBUG = true;

	//template <typename... ARGS>
	void LogState(std::string args) {
		if( !DEBUG )
			return;

		std::cout << args;
		std::cout << std::endl;
	}
}

bool operator<(const ENetAddress& lhs, const ENetAddress& rhs) {
	if( lhs.host == rhs.host )
		return lhs.port < rhs.port;

	return lhs.host < rhs.host;
}

class OtherEndpoint {

};

class Peer {

};

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

	ENetHost* createClient() {
		auto client = enet_host_create( nullptr, // create a client host
			                            31, // allowed outgoing connections
			                            2, // allow up 2 channels to be used, 0 and 1
			                            0, // any downstream bandwidth
			                            0  // any upstream bandwidth
		);

		if( client == nullptr )
			throw std::runtime_error("An error occurred while trying to create an ENet client.");

		return client;
	}
}

Network::Network( enet_uint16 incomingPort )
{
	_queues[0] = std::unique_ptr<std::deque<queue_element_type>>(new std::deque<queue_element_type>());
	_queues[1] = std::unique_ptr<std::deque<queue_element_type>>(new std::deque<queue_element_type>());

	_atomicQueues[0] = _queues[0].get();
	_atomicQueues[1] = _queues[1].get();

	if( enet_initialize() != 0 )
		throw std::runtime_error("Could not initialize ENET.");

	try {
		_server = createServer( incomingPort );
		_client = createClient( );
	} catch( ... ) {
		auto eptr = std::current_exception();
		// Invoke cleanup
		Destroy();
		std::rethrow_exception(eptr);
	}
}

void Network::Destroy() {
	_client = nullptr;
	_server = nullptr;
	enet_deinitialize();
}

Network::~Network() {
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

void Network::Peer::Send(buffer_type buffer, std::function<void()> callback) {
	auto packet = _createPacket(buffer);

	enet_peer_send(enetPeer, 0, packet);
	std::async(std::launch::async, callback);
}

void Network::SetConnectCallback(std::function<void(std::shared_ptr<hack::net::Network::Peer>)> callback) {
	_connectCallback = callback;
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

Network::Peer::Peer(ENetAddress address) :
	enetPeer(nullptr),
	address(std::forward<ENetAddress>(address))
{
}

Network::Peer::~Peer() {

	if( enetPeer == nullptr )
		return; //already cleaned up

	enet_peer_reset( enetPeer );
}

bool Network::Peer::operator <(const Peer& other) const {
	if( address.host < other.address.host )
		return true;
	else if( address.host == other.address.host )
		return address.port < other.address.port;

	return false;
}

bool Network::Peer::operator ==(const Peer& other) const {
	return GetUUID() == other.GetUUID();
}

void Network::Peer::Disconnect() {
	enet_peer_disconnect( enetPeer, 0 );
}

void Network::Peer::Receive(const ENetPacket& packet) {
	buffer_type data;
	data.reserve(packet.dataLength);
	data.assign(packet.data, packet.data + packet.dataLength);
	receiveCallback(data);
}

const std::string& Network::Peer::GetUUID() const {
	return _uuid;
}

void Network::Peer::SetUUID(std::string uuid) {
	_uuid = uuid;
}

void Network::CreatePeer( ENetPeer& peer ) {
	// We have to build Peer manually on heap here because we only allow Network to
	// instantiate Peer objects
	auto sharedPeer = std::shared_ptr<Peer>( new Peer( peer.address ) );

	auto insertPair = _peers.connected.insert( std::make_pair( peer.address, sharedPeer ) );

	if( insertPair.second )
		return; // Already connected?

	_peers.unconnected.erase( peer.address);

	peer.data = &(*insertPair.first);
	_connectCallback( sharedPeer );
}

void Network::HandleUnconnected() {

	//
	// Do connection handling
	//
	typedef decltype(_peers.unconnected) connections_type;

	connections_type outstandingConnections;
	{
		// Lock outstanding for use
		std::lock_guard<std::mutex> lock( _peers.unconnectedLock );

		outstandingConnections.swap( _peers.unconnected );
	}

	ENetEvent event;
	for( auto& entry : outstandingConnections ) {

		DEBUG.LOG_ENTRY(std::stringstream() << "Connecting to " << entry.host << ':' << entry.port);
		// Initiate the connection, allocating the two channels 0 and 1. */
		auto peer = enet_host_connect( _client, &entry, 2, 0);

		if (peer == nullptr) {
		   std::cerr << "CONNECT FAILED" << std::endl;
		   exit (-1);
		}

		// Wait 5 seconds for the connection attempt to succeed.
		if( enet_host_service( _client, &event, 5000) > 0
		 && event.type == ENET_EVENT_TYPE_CONNECT ) {
			DEBUG.LOG_ENTRY(std::stringstream() << "Connecting to " << entry.host << ':' << entry.port << " !SUCCEEDED!");
			CreatePeer( *peer );
		}
		else
		{
			/* Either the 5 seconds are up or a disconnect event was */
			/* received. Reset the peer in the event the 5 seconds   */
			/* had run out without any significant event.            */
			enet_peer_reset( peer );
			DEBUG.ERR_ENTRY(std::stringstream() << "Connecting to " << entry.host << ':' << entry.port << " !FAILED!");
		}
	}
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
				DEBUG.LOG_ENTRY( std::stringstream() << "Peer connected from "
				                 << event.peer->address.host << ':'
				                 << event.peer->address.port);

				CreatePeer( *event.peer );
				break;
			}
			case ENET_EVENT_TYPE_RECEIVE: {
				auto peer = extractPeer();
				DEBUG.LOG_ENTRY( std::stringstream() << "Packet received." << std::endl
				                  << "\tLength: " << event.packet->dataLength
				                  << "\tFrom: " << event.peer->address.host << ":" << event.peer->address.port);

				peer->Receive(*event.packet);

				// Clean up the packet now that we're done using it.
				enet_packet_destroy(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT: {
				auto peer = extractPeer();
				event.peer->data = nullptr;

				std::cout << "A client disconnected: " << peer->address.host
						  << ":" << peer->address.port
						  << std::endl;

				// enet code did clean itself up
				peer->enetPeer = nullptr;

				// We set the user data of Event to null so
				// we are allowed to remove the shared_ptr
				_peers.connected.erase(peer->address);
				_peers.unconnected.insert(peer->address);
				_disconnectCallback(*peer);
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

	auto& currentQueue = *_atomicQueues[1].load();
	//TODO: introduce locking
	// Process everything we have in the queue
	for( auto& element : currentQueue ) {
		if( element.peer )
			// Send to peer
			element.peer->Send( element.buffer, element.callback );
		else
			// Do broadcast
			Send( element.buffer, element.callback );
	}

	return true;
}

void Network::ExecuteWorker() {
	static const std::chrono::milliseconds duration( 1 );

	DEBUG.LOG_ENTRY("[Worker] Start...");

	while( _ExecuteWorker() ) {
		auto queue = _atomicQueues[0].load();
		if( queue == nullptr )
			// Should never happen but paranoia is so sweet
			continue;

		if( queue->empty() )
			// If there is nothing to do - do not spam
			// the CPU
			std::this_thread::sleep_for( duration );
	}

	DEBUG.LOG_ENTRY("[Worker] ...End");
}

void Network::ConnectTo( const std::string& host, enet_uint16 port ) {
	ENetAddress address;

	// Connect to other peer
	if( enet_address_set_host( &address, host.c_str() ) != 0 ) {
		throw std::runtime_error("Could no set address");
	}
	address.port = port;

	// Make sure we are the only thread accessing object
	std::lock_guard<std::mutex> lock( _peers.unconnectedLock );
	_peers.unconnected.insert( std::move(address) );
}

const std::string&
Network::Debug::GetCategory() const {
	static const std::string CATEGORY = "Network";
	return CATEGORY;
}
