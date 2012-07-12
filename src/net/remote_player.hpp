/*
 * remote_player.hpp
 *
 *  Created on: Jun 15, 2012
 *      Author: andreas
 */

#ifndef REMOTE_PLAYER_HPP_
#define REMOTE_PLAYER_HPP_

#include <queue>
#include "network.hpp"
#include "endpoint.hpp"
#include "../logic/player.hpp"
#include "../logic/serializable.hpp"

namespace hack {
namespace net {

class RemotePlayer : public hack::logic::Player, public Endpoint {
	void Deserialize(std::istream& stream);

	static const std::string NAME;
	std::weak_ptr<Network> _network;
	std::queue<buffer_type> _receiveQueue;
	std::function<void(buffer_type)> _receiver;
	std::weak_ptr<Network::Peer> _peer;
public:
	RemotePlayer(std::weak_ptr<Network> network, std::shared_ptr<Network::Peer> peer, std::string name);
	~RemotePlayer();
	bool operator==(const Network::Peer& peer) const;

	template <typename T>
	void SendTo(const T& buffer) {
		auto peer = _peer.lock();

		if( !peer )
			return;

		peer->Send( buffer );
	}
	std::queue<buffer_type> GetFrom() override;
	void Commit();
	const std::string& GetUUID() const override;
	bool IsProcessLocal() const override;
	const std::string& ClassName() const override;
};

bool operator ==(const Network::Peer& peer, const RemotePlayer& player);

} }


#endif /* REMOTE_PLAYER_HPP_ */
