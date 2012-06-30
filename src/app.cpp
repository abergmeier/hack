/*
 * main.cpp
 *
 *  Created on: Jun 18, 2012
 *      Author: andreas
 */

#include <future>
#include <memory>
#include <sstream>
#include "logic/objects.hpp"
#include "net/network.hpp"
#include "state/local_player.hpp"
#include "logic/stone.hpp"
#include "logic/objects.hpp"
#include "net/remote_player.hpp"
#include "logic/states.hpp"
#include "debug.hpp"
#include "net/registration.hpp"

namespace {
	class Debug : public hack::Debug {
		const std::string& GetCategory() const {
			static const std::string CATEGORY = "MAIN";
			return CATEGORY;
		}
	};

	static const Debug DEBUG;
}

int main( int argc, char** args ) {

	// Ignore binary path
	--argc;
	++args;


	std::set<std::shared_ptr<hack::logic::Player>> _players;

	hack::logic::Objects::Register<hack::logic::Stone>();
	auto stone = std::make_shared<hack::logic::Stone>();
	hack::logic::Objects::Get().Register(stone);

	//std::string name = "Andreas";
	auto sharedLocalPlayer = std::make_shared<hack::state::LocalPlayer>();
	_players.insert( sharedLocalPlayer );

	hack::net::Network network;
	// Connect to all known Peers before we register ourselves
	// so we do not have to filter ourselves
	for( auto& other : hack::net::Registration::GetAll() ) {
		network.ConnectTo(other.host, other.port );
	}

	hack::net::Registration registration( sharedLocalPlayer->GetUUID(), network.GetIncomingPort() );

	auto& objects = hack::logic::Objects::Get();
	auto& states = hack::state::States::Get();
	states.SetNetwork( network );

	auto playerConnected = [&_players, &states](std::shared_ptr<hack::net::Network::Peer> peer) {
		auto shared = std::make_shared<hack::net::RemotePlayer>(peer, "Unnamed");

		auto sharedPlayer = std::static_pointer_cast<hack::logic::Player>( shared );

		for( auto& otherPlayer : _players ) {
			DEBUG.LOG_ENTRY(std::stringstream() << "Sending player " << otherPlayer->GetName() << " to " << sharedPlayer->GetName() );
			states.CommitTo( *otherPlayer, sharedPlayer );
		}

		_players.insert(shared);

#if 0
		for( const auto& object : objects ) {
			peer->Send(object);
		}
#endif
	};

	auto playerDisconnected = [&_players](hack::net::Network::Peer& peer) {

		for( auto it = _players.cbegin(); it != _players.cend(); ++it ) {
			auto remotePlayerPtr = std::dynamic_pointer_cast<hack::net::RemotePlayer>(*it);

			if( remotePlayerPtr == nullptr )
				continue; // Not a RemotePlayer

			if( *remotePlayerPtr == peer ) {
				_players.erase(it);
				break;
			}
		}
	};

	network.SetConnectCallback(playerConnected);
	network.SetDisconnectCallback(playerDisconnected);

	auto actions = [&states, &objects]() {
		static const std::chrono::milliseconds duration( 5000 );

		// If there is nothing to do - do not spam
		// the CPU
		std::this_thread::sleep_for( duration );
		for( auto& object : objects ) {
			auto sharedObject = object.lock();

			if( !sharedObject )
				continue;
			states.Commit( *sharedObject );
		}
	};

	const std::launch policy = std::launch::async;
	auto future = std::async(policy, actions);

	network.ExecuteWorker();
#if 0
	// Start all subsystems asynchronous
	std::vector<std::future<void>> futures;


	futures.push_back(std::async(policy, network.ExecuteWorker));

	futures.push_back(std::async(policy, logic.ExecuteWorker()));
	futures.push_back(std::async(policy, ui.ExecuteWorker()));

	// Wait for subsystems to terminate
	for( auto& future : futures ) {
		future.wait();
	}
#endif

/*
	auto& network = hack::net::Network::Get();
	network.AddPeer("127.0.0.1", 5647);
	auto future = std::async(std::launch::async, network.ExecuteWorker());
	future.wait();
*/
}

