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

int main( int argc, char** args ) {

	enet_uint16 incomingPort = 50123;

	// Ignore binary path
	--argc;
	++args;

	if( argc >= 1 ) {
		// Handle incoming port
		std::stringstream parser(*args);
		parser >> incomingPort;

		--argc;
		++args;
	}

	std::set<std::shared_ptr<hack::logic::Player>> _players;

	hack::logic::Objects::Register<hack::logic::Stone>();

	//std::string name = "Andreas";
	_players.insert(std::make_shared<hack::state::LocalPlayer>());

	hack::net::Network network( incomingPort );
	for( ; argc > 0; --argc, ++args ) {
		enet_uint16 peerPort = 0;
		std::stringstream parser(*args);
		parser >> peerPort;
		network.ConnectTo("localhost", peerPort);
	}

	auto playerConnected = [&_players](std::shared_ptr<hack::net::Network::Peer> peer) {
		auto shared = std::make_shared<hack::net::RemotePlayer>(peer, "Unnamed");

		auto& states = hack::state::States::Get();
		for( auto& otherPlayer : _players ) {
			states.CommitTo( *otherPlayer, *shared );
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

	network.ExecuteWorker();
#if 0
	// Start all subsystems asynchronous
	std::vector<std::future<void>> futures;
	const std::launch policy = std::launch::async;

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

