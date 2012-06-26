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
// Undef because defines on Windows fuck up seriously
#undef noexcept
#include <enet/enet.h>
#ifdef _MSC_VER
#define noexcept
#endif //_MSC_VER

#include <future>
#include "../subsystem.hpp"

extern bool operator<(const ENetAddress& lhs, const ENetAddress& rhs);

namespace hack {
namespace net {

class Network : public hack::Subsystem {
public:
	typedef std::vector<enet_uint8> buffer_type;

	Network( enet_uint16 incomingPort );
	class Peer {
		// Make sure we can only be created by the network
		friend class Network;
		Peer(ENetAddress address);
		void Destroy();
		std::queue<std::tuple<buffer_type, std::function<void()>>> sendQueue;
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
		void Send(buffer_type buffer, std::function<void()> callback);
		const std::string& GetUUID() const;

		const ENetAddress address;
	};

private:
	std::function<void(std::shared_ptr<Peer>)> _connectCallback;
	std::function<void(hack::net::Network::Peer&)> _disconnectCallback;
	void Destroy();
	void CreatePeer( ENetPeer& event );

	struct DEBUG {
		static std::ostream& LOG      ( std::ostream& stream );
		static std::ostream& LOG      ( const std::string& str );
		static std::ostream& ERR      ( std::ostream& stream );
		static void         LOG_ENTRY( std::ostream& stream);
		static void         LOG_ENTRY( const std::string& str);
		static void         ERR_ENTRY( std::ostream& stream);
	};

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

	ENetHost* _client, * _server;

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
	virtual ~Network() noexcept;
	void Setup();

	// Processes the queue indefinitely
	void ExecuteWorker();

	template <typename T>
	void Send(const T& buffer, std::function<void()> callback) {
		auto packet = _createPacket(buffer);
		enet_host_broadcast(_client, 0, packet);
		std::async(std::launch::async, callback);
	}

	template <typename T>
	void Send(const T& buffer) {
		Send(buffer, []() {});
	}


	// Callback is executed synchronously. Make sure it returns very fast to not
	// block the network communication!
	void SetReceiveCallback(std::function<void(buffer_type)> callback);
	void SetConnectCallback(std::function<void(std::shared_ptr<hack::net::Network::Peer>)> callback);
	void SetDisconnectCallback(std::function<void(hack::net::Network::Peer&)> callback);
	void ConnectTo( const std::string& host, enet_uint16 port );
private:

	// Processes a filled queue
	bool _ExecuteWorker();
	// Process all known peers that are not yet connected
	void HandleUnconnected();


};

} }


#endif /* NETWORK_HPP_ */
