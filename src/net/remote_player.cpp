
//#include <Poco/Base64Decoder.h>
#include "remote_player.hpp"
#include "../include.hpp"

using namespace hack::net;

const std::string RemotePlayer::NAME("RemotePlayer");

RemotePlayer::RemotePlayer(std::shared_ptr<Network::Peer> peer, std::string name) :
	hack::logic::Player(name),
	_receiveQueue(),
	_receiver([](buffer_type buffer) {
		//_receiveQueue.push(std::forward<buffer_type>(buffer));
	}),
	_peer(peer)
{
	peer->receiveCallback = _receiver;
}

const std::string& RemotePlayer::GetUUID() const {
	auto peer = _peer.lock();

	if( peer )
		return peer->GetUUID();

	throw std::runtime_error("Cannot retrieve UUID - Peer is no longer connected");
}

RemotePlayer::~RemotePlayer() {
	auto peer = _peer.lock();

	if( peer )
		peer->receiveCallback = nullptr;
}

bool
RemotePlayer::operator ==(const hack::net::Network::Peer& peer) const {
	auto sharedPeer = _peer.lock();

	return (*sharedPeer) == peer;
}

bool
hack::net::operator ==(const Network::Peer& peer, const RemotePlayer& player) {
	// Just redirect to Player implementation
	return player == peer;
}

std::queue<RemotePlayer::buffer_type> RemotePlayer::GetFrom() {
	return std::move(_receiveQueue);
}

void RemotePlayer::Deserialize(std::istream& stream) {
//	Poco::Base64Decoder decoder(stream);
}

void RemotePlayer::Commit() {
	//TODO: commit to be displayed
}

std::ostream& RemotePlayer::SerializeContent(std::ostream& stream) const {
	return Serializable::SerializeContent( NAME, stream );
}

bool RemotePlayer::IsProcessLocal() const {
	return false;
}

