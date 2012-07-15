/*
 * network.hpp
 *
 *  Created on: Jun 15, 2012
 *      Author: andreas
 */

#ifndef NETWORK_HPP_
#define NETWORK_HPP_

//disable insignificant compiler warning under vs2012 compiler
//this problem only occurs in visual studio
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable: 4250)
#endif // _MSC_VER

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
#include <sstream>
#include <iomanip>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketNotification.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketAcceptor.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/AutoPtr.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/NObserver.h>
#include <Poco/Net/NetException.h>
#include <Poco/Exception.h>
#ifdef _MSC_VER 
#ifndef WIN32
#define WIN32
#endif
#endif
#include <future>
#include "../subsystem.hpp"
#include "../debug.hpp"

//enable compiler warnings again
#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER

namespace hack {
namespace net {

using namespace Poco::Net;

class Network : public hack::Subsystem {
public:
	typedef std::string packet_type;
	typedef std::string buffer_type;

	struct Address : public Poco::Net::SocketAddress {
		Address( const std::string& host, Poco::UInt16 port, std::string uuid );
		Address( const SocketAddress&, std::string uuid );
		Address( Address&& other );
		Address& operator=( Address&& other );
		bool operator < (const Address& other) const;
		const std::string& GetUUID() const;
	private:
		std::string uuid;
		friend class Network;
	};

	struct Peer {
		~Peer();
		bool operator < (const Peer& other) const;
		bool operator ==(const Peer& other) const;

		const std::string uuid;
		StreamSocket& GetSocket();
		std::function<void(buffer_type)> GetReceiveCallback();
		void SetReceiveCallback( std::function<void(buffer_type)> func );
	private:
		// Make sure we can only be created by the network
		friend class Network;
		Peer( Network& network, StreamSocket& socket, SocketReactor& reactor, std::string uuid, std::function<void()> connectedCallback );
		void Destroy();
		//std::queue<std::tuple<buffer_type, std::function<void()>>> sendQueue;

		StreamSocket   _socket;
		SocketReactor& _reactor;
		Network&       _network;
		std::function<void(buffer_type)> _receiveCallback;
		// Gets called once the Peer is connected in both
		// directions
		std::function<void()> _connectedCallback;
	};

	friend struct Peer;
private:

	class PeerWrapper {
		std::shared_ptr<Network::Peer> _peer;
		StreamSocket   _socket;
		SocketReactor& _reactor;
		Network&       _network;
	public:
		void OnReadable ( const Poco::AutoPtr<ReadableNotification>& );
		void OnWriteable( const Poco::AutoPtr<WritableNotification>& );
		void OnShutdown ( const Poco::AutoPtr<ShutdownNotification>& );
		void OnTimeout  ( const Poco::AutoPtr<TimeoutNotification>& );
		void OnError    ( const Poco::AutoPtr<ErrorNotification>&   );

		PeerWrapper(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor);
		PeerWrapper(Network& network, Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor);

		~PeerWrapper();
	};

	friend class PeerWrapper;

	static const unsigned int MAX_PACKET_LENGTH;

	template <typename T>
	static packet_type _createPacket(const T& buffer) {
		std::stringstream stream;
		Poco::UInt32 length = buffer.length();
		// 10 chars should be the max output since 4.294.967.295 is
		// the max value
		stream << std::setw(MAX_PACKET_LENGTH) << std::setfill('0') << length << '\n' << buffer;
		return stream.str();
	}

	template <typename T>
	static void _SendTo( StreamSocket& socket, const T& buffer ) {
		auto packet = _createPacket( buffer );
		SendPacket( socket, packet );
	}

	std::function<void(std::shared_ptr<Peer>)>            _connectCallback;
	std::function<void(const std::string&, Poco::UInt32)> _connectFailedCallback;
	std::function<void(hack::net::Network::Peer&)>        _disconnectCallback;
	std::shared_ptr<Peer> CreatePeer( StreamSocket& socket, SocketReactor& reactor, std::string uuid, std::function<void()> connectedCallback );

	struct queue_element_type {
		// Destination of data - broadcast if peer is null
		std::weak_ptr<Peer> peer;
		packet_type         packet;
	};

	struct {
		mutable std::recursive_mutex lock;
		std::deque<queue_element_type> input;
	} _queues;

	struct Peers {
		// Mutex to be used when object is accessed
		mutable std::recursive_mutex lock;

		// All known addresses of other peers
		std::set<Address> unconnected;
#if 0
		//
		std::map<StreamSocket, std::string> awaitingConnection;
#endif
		// Not fully connected peers, that miss the handshake
		// Use this special, mostly small or empty map for performance purposes.
		std::map<StreamSocket, std::string> awaitingHandshake;

		// All peers which are connected
		std::map<Address, std::shared_ptr<Peer>> connected;
#if 0
		void AbortWait( const StreamSocket& socket );
#endif
	};

	struct Acceptor : public Poco::Net::SocketAcceptor<PeerWrapper> {
		Network& network;
		Acceptor( Network& network, Poco::Net::ServerSocket& socket, Poco::Net::SocketReactor& reactor ) :
			Poco::Net::SocketAcceptor<PeerWrapper>(socket, reactor),
			network(network)
		{
		}

		PeerWrapper* createServiceHandler( Poco::Net::StreamSocket& socket ) override {
			auto wrapper = new PeerWrapper( network, socket, *reactor() );
			network.OnConnect( socket, *reactor() );
			return wrapper;
		}
	};

	Peers _peers;
	std::string _ipAddress;

	SocketReactor _reactor;
	std::unique_ptr<ServerSocket> _server;
	std::unique_ptr<Acceptor> _acceptor;

	void onReadable ( const Poco::AutoPtr<ReadableNotification>& notifiction );

	// Processes a filled queue
	bool _ExecuteWorker();
	// Process all known peers that are not yet connected
	void ConnectOutstanding();

	// Send all previously unsent packets
	void HandleUnsent();

	void OnConnect( StreamSocket& socket, SocketReactor& reactor );
	void OnTimeout( const Poco::AutoPtr<TimeoutNotification>& );
	void OnDisconnect( StreamSocket& socket );

	bool IsConnecting( const std::string& uuid ) const;
	bool IsConnected ( const std::string& uuid ) const;

	void HandleTimeout();
	std::shared_ptr<Network::Peer> FinishHandshake( StreamSocket& socket, SocketReactor& reactor, std::string otherUuid, bool isSlave );

	// Immediately sends Packet to all connected Peers

	void SendPacket( const packet_type& packet );
	// Immediately sends Packet to Peer
	static void SendPacket( StreamSocket& socket, const packet_type& packet );

	void Enqueue( std::weak_ptr<Peer> peer, packet_type packet );
public:
	Network( std::string uuid );

	virtual ~Network();

	// Processes the queue indefinitely
	void ExecuteWorker() override;

	template <typename T>
	void SendTo(const T& buffer) {
		Enqueue( std::shared_ptr<Peer>(), _createPacket( buffer ) );
	}

	template <typename T>
	void SendTo( std::shared_ptr<Peer> peer, const T& buffer) {
		Enqueue( peer, _createPacket( buffer ) );
	}

	bool WaitUntilConnected( const std::string& uuid ) const;

	// Callback is executed synchronously. Make sure it returns very fast to not
	// block the network communication!
	void SetReceiveCallback(std::function<void(buffer_type)> callback);
	void SetConnectCallback(std::function<void(std::shared_ptr<hack::net::Network::Peer>)> callback);
	void SetConnectFailedCallback(std::function<void(const std::string&, size_t)> callback);
	void SetDisconnectCallback(std::function<void(hack::net::Network::Peer&)> callback);
	bool ConnectTo( const std::string& host, Poco::UInt32 port, std::string uuid );
	std::string GetIPAddress() const;
	Poco::UInt32 GetIncomingPort() const;
	const std::string uuid;
protected:
	void StopWorker() override;
};

} }


#endif /* NETWORK_HPP_ */
