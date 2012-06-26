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
public:
	RemotePlayer(std::shared_ptr<Network::Peer> peer, std::string name);
	~RemotePlayer();
	bool operator==(const Network::Peer& peer) const;
	void SendTo(buffer_type buffer, std::function<void()> callback) override;
	std::queue<buffer_type> GetFrom() override;
	void Commit();
	const std::string& GetUUID() const;
protected:
	std::ostream& SerializeContent(std::ostream& stream) const;
private:
	void Deserialize(std::istream& stream);

	static const std::string NAME;
	std::queue<buffer_type> _receiveQueue;
	std::function<void(buffer_type)> _receiver;
	std::weak_ptr<Network::Peer> _peer;
};

bool operator ==(const Network::Peer& peer, const RemotePlayer& player);

} }


#endif /* REMOTE_PLAYER_HPP_ */