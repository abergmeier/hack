
//#include <Poco/Base64Decoder.h>
#include "remote_player.hpp"
#include "../include.hpp"
#include <string.h>
#include "../state/states.hpp"
#include "../debug.hpp"

using namespace hack::net;

namespace {
	struct Debug : public hack::Debug {
		const std::string& GetCategory() const override {
			static const std::string CATEGORY = "RemotePlayer";
			return CATEGORY;
		}
	};

	const Debug DEBUG;
}

const std::string RemotePlayer::NAME("RemotePlayer");

const std::string&
RemotePlayer::ClassName() const {
	return NAME;
}

RemotePlayer::RemotePlayer(std::weak_ptr<Network> network, std::weak_ptr<hack::state::States> states, std::shared_ptr<Network::Peer> peer, std::string name) :
	hack::logic::Player(name),
	_network     ( network ),
	_receiveQueue(),
	_states(states),
	_receiver([&](buffer_type buffer) {
		// Transforms network to deserializable format

		auto sharedStates = _states.lock();

		if( !sharedStates )
			return;
		// Forward this to objects
		sharedStates->ReceiveFrom( std::move(buffer), *this );
	}),
	_peer(peer)
{
	peer->SetReceiveCallback( _receiver );
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
		peer->SetReceiveCallback( nullptr );
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

