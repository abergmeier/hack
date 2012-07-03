/*
 * main.cpp
 *
 *  Created on: Jun 18, 2012
 *      Author: andreas
 */

#include <future>
#include <memory>
#include <vector>
#include <sstream>
#include <iostream>
#include "logic/objects.hpp"
#include "net/network.hpp"
#include "state/local_player.hpp"
#include "logic/stone.hpp"
#include "logic/objects.hpp"
#include "net/remote_player.hpp"
#include "logic/states.hpp"
#include "graphics/renderer.hpp"
#include "debug.hpp"
#include "net/registration.hpp"
#include "logic/avatar.hpp"

namespace {
	class Debug : public hack::Debug {
		const std::string& GetCategory() const {
			static const std::string CATEGORY = "MAIN";
			return CATEGORY;
		}
	};

	const Debug DEBUG;

	void RegisterAllClasses() {
		hack::logic::Objects::Register<hack::logic::Stone>();
		hack::logic::Objects::Register<hack::logic::Avatar>();
	}
}

using namespace hack::logic;

int main() {

	std::set<std::shared_ptr<Player>> _players;

	RegisterAllClasses();

	//std::string name = "Andreas";
	auto sharedLocalPlayer = std::make_shared<hack::state::LocalPlayer>();
	_players.insert( sharedLocalPlayer );

	auto stone = std::make_shared<Stone>();
	hack::logic::Objects::Get().Register(stone);

	auto playerAvartar = std::make_shared<Avatar>();

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

#if 0
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
#endif
	const auto policy = std::launch::async;
#if 0
	auto future = std::async(policy, actions);
#endif

	{
		// Start all subsystems asynchronous
		std::vector<std::pair<std::future<void>, hack::Subsystem*>> futures;

		{ // Start Registration
			auto worker = std::bind(&hack::net::Registration::ExecuteWorker, std::ref(registration));
			futures.push_back(std::make_pair(std::async(policy, worker), &registration));
		}

		{ // Start Networking
			auto worker = std::bind(&hack::net::Network::ExecuteWorker, std::ref(network));
			futures.push_back(std::make_pair(std::async(policy, worker), &network));
		}


#if 0
		futures.push_back(std::async(policy, logic.ExecuteWorker()));
		futures.push_back(std::async(policy, ui.ExecuteWorker()));
#endif

		renderer r(640,480);

		auto mover = [playerAvartar](float x, float y) {
			playerAvartar->setX( std::lround(x) );
			playerAvartar->setY( std::lround(y) );
		};

		auto rotater = [playerAvartar](float rot) {
			playerAvartar->setAngle( rot );
		};

		auto attacker = [playerAvartar]() {
			//TODO: Implement attack
		};

		hack::logic::Stone s;
		r.registerEntity(s,hack::logic::Stone::NAME.c_str());
		r.run();
		// Runs till window is closed

		for( auto& future : futures ) {
			future.second->StopWorker();
		}
		// Wait for subsystems to terminate
		for( auto& future : futures ) {
			future.first.wait();
		}
	}

/*
	auto& network = hack::net::Network::Get();
	network.AddPeer("127.0.0.1", 5647);
	auto future = std::async(std::launch::async, network.ExecuteWorker());
	future.wait();
*/
}

