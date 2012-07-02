/*
 * network.hpp
 *
 *  Created on: Jun 15, 2012
 *      Author: andreas
 */

#ifndef NETWORK_HPP_
#define NETWORK_HPP_

#include <atomic>
#include <memory>
#include <map>
#include <array>
#include <deque>
#include <queue>
#include <functional>
#include <tuple>
#include <set>
#include <istream>
#include <ostream>
#include <mutex>
//#include <Poco/Net/SocketAddress.h>
#ifdef _MSC_VER 
#ifndef WIN32
#define WIN32
#endif
#endif
#include <enet/enet.h>
#include <future>
#include "../subsystem.hpp"
#include "../debug.hpp"

extern bool operator<(const ENetAddress& lhs, const ENetAddress& rhs);

namespace hack {
namespace net {

class Network : public hack::Subsystem {
	template <typename T>
	static ENetPacket* _createPacket(const T& buffer) {
		const size_t byteCount = buffer.size() * sizeof(typename T::value_type);
		auto packet = enet_packet_create( buffer.data(),
		                                  byteCount,
		                                  ENetPacketFlag::ENET_PACKET_FLAG_RELIABLE
		);

		if( packet == nullptr )
			throw std::runtime_error("Could not create packet!");
		return packet;
	}
public:
	typedef std::vector<enet_uint8> buffer_type;

	Network( );
	class Peer {
		// Make sure we can only be created by the network
		friend class Network;
		Peer( ENetPeer& peer );
		void Destroy();
		//std::queue<std::tuple<buffer_type, std::function<void()>>> sendQueue;
		std::string _uuid;
		ENetPeer* enetPeer;
		ENetPeer* GetPeer();
		void Disconnect();
		void Receive(const ENetPacket& packet);
		void SetUUID(std::string uuid);
	public:
		~Peer();
		bool operator < (const Peer& other) const;
		bool operator ==(const Peer& other) const;
		std::function<void(buffer_type)> receiveCallback;
		template <typename T>
		void Send(const T& buffer) {
			auto packet = _createPacket( buffer );
			if( enet_peer_send( enetPeer, 0, packet ) != 0 )
				throw std::runtime_error("SEND FAIL");
		}

		const std::string& GetUUID() const;

		const ENetAddress address;
	};

private:
	std::function<void(std::shared_ptr<Peer>)> _connectCallback;
	std::function<void(hack::net::Network::Peer&)> _disconnectCallback;
	void Destroy();
	void CreatePeer( ENetPeer& event );


	struct queue_element_type {
		// Destination of data - broadcast if peer is null
		std::shared_ptr<Peer> peer;
		buffer_type buffer;
		 std::function<void()> callback;
	};

	std::array<std::unique_ptr<std::deque<queue_element_type>>, 2> _queues;
	std::array<std::atomic<std::deque<queue_element_type>*>, 2> _atomicQueues;

	struct {
		// Mutex to be used when object is accessed
		std::mutex unconnectedLock;
		// All known addresses of other peers
		std::set<ENetAddress> unconnected;
		// All peers which are connected
		std::map<ENetAddress, std::shared_ptr<Peer>> connected;
	} _peers;

	ENetHost* _server;
	enet_uint16 _port;

public:
	virtual ~Network();
	void Setup();

	// Processes the queue indefinitely
	void ExecuteWorker();

	template <typename T>
	void Send(const T& buffer) {
		auto packet = _createPacket(buffer);
		enet_host_broadcast( _server, 0, packet );
	}

	// Callback is executed synchronously. Make sure it returns very fast to not
	// block the network communication!
	void SetReceiveCallback(std::function<void(buffer_type)> callback);
	void SetConnectCallback(std::function<void(std::shared_ptr<hack::net::Network::Peer>)> callback);
	void SetDisconnectCallback(std::function<void(hack::net::Network::Peer&)> callback);
	void ConnectTo( const std::string& host, enet_uint16 port );
	enet_uint16 GetIncomingPort() const;
private:

	// Processes a filled queue
	bool _ExecuteWorker();
	// Process all known peers that are not yet connected
	void HandleUnconnected();


};

} }


#endif /* NETWORK_HPP_ */
