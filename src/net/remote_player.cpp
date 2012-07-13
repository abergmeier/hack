
//#include <Poco/Base64Decoder.h>
#include "remote_player.hpp"
#include "../include.hpp"
#include <string.h>
#include "../logic/states.hpp"

using namespace hack::net;

const std::string RemotePlayer::NAME("RemotePlayer");

const std::string&
RemotePlayer::ClassName() const {
	return NAME;
}

RemotePlayer::RemotePlayer(std::weak_ptr<Network> network, std::shared_ptr<Network::Peer> peer, std::string name) :
	hack::logic::Player(name),
	_network     ( network ),
	_receiveQueue(),
	_receiver([this](buffer_type buffer) {
		// Transforms network to deserializable format

		// Forward this to objects
		hack::state::States::Get().ReceiveFrom( std::move(buffer), *this );
	}),
	_peer(peer)
{
	peer->receiveCallback = _receiver;
}

const std::string& RemotePlayer::GetUUID() const {
	auto peer = _peer.lock();

	if( peer )
		return peer->uuid;

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

bool RemotePlayer::IsProcessLocal() const {
	return false;
}

