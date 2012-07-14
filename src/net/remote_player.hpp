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
#include "../state/serializable.hpp"
#include "../state/states.hpp"

namespace hack {
namespace net {

class RemotePlayer : public hack::logic::Player, public Endpoint {
	void Deserialize(std::istream& stream);

	static const std::string NAME;
	std::weak_ptr<Network> _network;
	std::queue<buffer_type> _receiveQueue;
	std::function<void(buffer_type)> _receiver;
	std::weak_ptr<Network::Peer> _peer;
	std::weak_ptr<hack::state::States> _states;
public:
	RemotePlayer(std::weak_ptr<Network> network, std::weak_ptr<hack::state::States> states, std::shared_ptr<Network::Peer> peer, std::string name);
	~RemotePlayer();
	bool operator==(const Network::Peer& peer) const;

	template <typename T>
	void SendTo( const T& buffer ) {
		auto peer = _peer.lock();

		if( !peer )
			return;

		auto sharedNetwork = _network.lock();

		if( !sharedNetwork )
			return;

		sharedNetwork->SendTo( peer, buffer );
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
