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
#include <future>
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
public:
	typedef std::vector<enet_uint8> buffer_type;

	struct Address : public ENetAddress {
		Address( const std::string& host, enet_uint16 port, std::string uuid );
		Address( ENetAddress address, std::string uuid );
		Address( Address&& other );
		Address& operator=( Address&& other );
		std::string uuid;
		bool operator < (const Address& other) const;
	};

	struct Peer {
		~Peer();
		bool operator < (const Peer& other) const;
		bool operator ==(const Peer& other) const;
		std::function<void(buffer_type)> receiveCallback;

		template <typename T>
		void Send(const T& buffer) {
			SendTo( enetPeer, buffer );
		}

		const std::string uuid;
		const ENetAddress address;
	private:
		// Make sure we can only be created by the network
		friend class Network;
		Peer( ENetPeer& peer, std::string uuid );
		void Destroy();
		//std::queue<std::tuple<buffer_type, std::function<void()>>> sendQueue;

		ENetPeer* enetPeer;
		ENetPeer* GetPeer();
		void Disconnect();
		void Receive(const ENetPacket& packet);
	};
private:
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

	template <typename T>
	static void SendTo(ENetPeer* enetPeer, const T& buffer) {
		auto packet = _createPacket( buffer );
		if( enet_peer_send( enetPeer, 0, packet ) != 0 )
			throw std::runtime_error("SEND FAIL");
	}

	std::function<void(std::shared_ptr<Peer>)> _connectCallback;
	std::function<void(hack::net::Network::Peer&)> _disconnectCallback;
	void Destroy();
	void CreatePeer( ENetPeer& event, std::string uuid );

	struct queue_element_type {
		// Destination of data - broadcast if peer is null
		std::shared_ptr<Peer> peer;
		buffer_type buffer;
		 std::function<void()> callback;
	};

	std::array<std::unique_ptr<std::deque<queue_element_type>>, 2> _queues;
	std::array<std::atomic<std::deque<queue_element_type>*>, 2> _atomicQueues;

	struct Peers {
		// Mutex to be used when object is accessed
		mutable std::recursive_mutex lock;

		// All timeouts to the Peers
		std::map<ENetPeer*, std::future<void>> connectionTimeout;

		// All known addresses of other peers
		std::set<Address> unconnected;

		//
		std::map<ENetPeer*, std::string> awaitingConnection;

		// Not fully connected peers, that miss the handshake
		std::map<ENetPeer*, std::string> awaitingHandshake;

		// All peers which are connected
		std::map<Address, std::shared_ptr<Peer>> connected;

		static std::string ExtractUUID( ENetPacket* packet );
		void AbortWait( ENetPeer& peer );
	};

	Peers _peers;

	ENetHost* _server;
	enet_uint16 _port;

	enum {
		STOPPED,
		RUNNING,
		HALTING
	} _state;

	// Processes a filled queue
	bool _ExecuteWorker();
	// Process all known peers that are not yet connected
	void HandleUnconnected();

	bool IsConnecting( const std::string& uuid ) const;
	bool IsConnected ( const std::string& uuid ) const;

	void HandleTimeout();
public:
	Network( std::string uuid );

	virtual ~Network();
	void Setup();

	// Processes the queue indefinitely
	void ExecuteWorker() override;
	void StopWorker() override;

	template <typename T>
	void Send(const T& buffer) {
		auto packet = _createPacket(buffer);
		enet_host_broadcast( _server, 0, packet );
	}

	bool WaitUntilConnected( const std::string& uuid ) const;

	// Callback is executed synchronously. Make sure it returns very fast to not
	// block the network communication!
	void SetReceiveCallback(std::function<void(buffer_type)> callback);
	void SetConnectCallback(std::function<void(std::shared_ptr<hack::net::Network::Peer>)> callback);
	void SetDisconnectCallback(std::function<void(hack::net::Network::Peer&)> callback);
	void ConnectTo( const std::string& host, enet_uint16 port, std::string uuid );
	enet_uint16 GetIncomingPort() const;
	const std::string uuid;
};

} }


#endif /* NETWORK_HPP_ */
