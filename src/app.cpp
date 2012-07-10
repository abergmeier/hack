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
#include <limits>
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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES // for C++
#include <math.h>
#endif // _MSC_VER


using hack::state::LocalPlayer;
using hack::net::Registration;
using hack::state::States;
using hack::net::RemotePlayer;
using hack::net::Network;
using hack::logic::vector2;
using namespace hack::graphics;
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

	void UpdateRotation(vector2<int>& mousePosition, hack::logic::Avatar& playerAvatar) {

		static const vector2<int> ORIG_ROTATION(0, -1);
		static const auto         ORIG_LENGTH = ORIG_ROTATION.length();

		vector2<int> vector(
			mousePosition[0] - playerAvatar.getX(),
			mousePosition[1] - playerAvatar.getY()
		);

		//DEBUG.LOG_ENTRY(std::stringstream() << "length " << vector.dot( ORIG_ROTATION ) << " " << vector.length() << ' ' << ORIG_LENGTH);

		const auto vectorLength = vector.length();

		// If mouse is in avatar center there is no
		// point in changing angle
		if( vectorLength == 0.0 )
			return;

		auto cosrot = vector.dot( ORIG_ROTATION ) / ( vectorLength * ORIG_LENGTH );
		auto rot = std::acos( cosrot ) / M_PI * 180.0f;
		

		// Since rotation calculation only works in [0, PI]
		// we have to process the direction of rotation
		// separately
		if( vector[0] < 0 )
			rot = -rot;

		playerAvatar.setAngle( rot );
	};

	static const int WINDOW_WIDTH = 640;
	static const int WINDOW_HEIGHT = 480;

	vector2<int> lastMousePosition;

	static const auto ASYNC_POLICY = std::launch::async;

	std::function<void(int x, int y)> getAvatarMoveHandler( std::shared_ptr<hack::logic::Avatar> sharedAvatar,  hack::logic::Objects &obj) {
		auto& lastMousePosition = ::lastMousePosition;
		return [&lastMousePosition, sharedAvatar, &obj]( int x, int y ) {

			int xx = sharedAvatar->getX() + x;
			int yy = sharedAvatar->getY() + y;

			if (obj.movementCheck(*sharedAvatar)) {
				sharedAvatar->setX(xx);
				sharedAvatar->setY(yy);
			}
			//DEBUG.LOG_ENTRY(std::stringstream() << "Avatar Pos: " << sharedAvatar->getX() << ':' << sharedAvatar->getY());

			UpdateRotation( lastMousePosition, *sharedAvatar );
		};
	}

	std::function<void(int x, int y)> getMouseMoveHandler( std::shared_ptr<hack::logic::Avatar> sharedAvatar ) {
		auto& lastMousePosition = ::lastMousePosition;
		return [&lastMousePosition, sharedAvatar]( int absx, int absy ) {
			// Save for further processing
			lastMousePosition[0] = absx;
			lastMousePosition[1] = absy;

			//DEBUG.LOG_ENTRY(std::stringstream() << "Mouse Pos: " << absx << ':' << absy);

			UpdateRotation( lastMousePosition, *sharedAvatar );
		};
	}

	std::function<void()> getAvatarAttackHandler( std::shared_ptr<hack::logic::Avatar> sharedAvatar ) {
		return [sharedAvatar]() {
			//TODO: Implement attack
		};
	}

	bool IsFirstNode( const hack::logic::Player& player, Network& network ) {

		std::map<Registration::Element::timestamp_type, std::string> timeMap;

		const auto& localUUID =  player.GetUUID();

		auto ourTime = std::numeric_limits<Registration::Element::timestamp_type>::max();

		// Create a map with key of all timestamps sorted
		for( auto& other : Registration::GetAll() ) {
			if( other.uuid == localUUID )
				ourTime = other.time;

			timeMap.insert( std::make_pair(other.time, other.uuid) );
		}

		if( timeMap.empty() )
			return true;

		auto it = timeMap.find( ourTime );

		// Process in reverse, so chance of
		// near timeout entries are less
		do {
			--it;
			if( network.WaitUntilConnected( (*it).second ) )
				return true;
		}
		while( it != timeMap.begin() );

		return false;
	}
}

using namespace hack::logic;

int main() {
	Objects objects;

	std::set<std::shared_ptr<Player>> _players;

	RegisterAllClasses();

	auto sharedLocalPlayer = std::make_shared<LocalPlayer>();
	_players.insert( sharedLocalPlayer );

	auto r = std::make_shared<renderer>(WINDOW_WIDTH, WINDOW_HEIGHT);
	objects.SetCallback( std::weak_ptr<renderer>(r) );

	auto network = std::make_shared<hack::net::Network>( sharedLocalPlayer->GetUUID() );
	// Connect to all known Peers before we register ourselves
	// so we do not have to filter ourselves

	const auto others = Registration::GetAll();

	for( auto& other : others ) {
		network->ConnectTo( other.host, other.port, other.uuid );
	}

	// Start all subsystems asynchronous
	std::vector<std::pair<std::future<void>, hack::Subsystem*>> futures;

	{ // Start Networking
		auto worker = std::bind(&hack::net::Network::ExecuteWorker, std::ref(network));
		futures.push_back(std::make_pair(std::async(ASYNC_POLICY, worker), network.get()));
	}

	// Before doing anything else, we first have to register with the Server
	Registration registration( sharedLocalPlayer->GetUUID(), network->GetIPAddress(), network->GetIncomingPort() );

	// Check whether we are the first node in the game
	// If we are, we have to create the objects
	const auto needObjectCreation = IsFirstNode( *sharedLocalPlayer, *network );

	// Setup states
	auto& states = States::Get();
	states.SetNetwork( network );
	states.SetDeserializer( [&objects]( std::istream& stream ) {
		objects.Deserialize( stream );
	});

	auto playerConnected = [&_players, &states](std::shared_ptr<hack::net::Network::Peer> peer) {
		auto shared = std::make_shared<RemotePlayer>(peer, "Unnamed");

		auto sharedPlayer = std::static_pointer_cast<Player>( shared );

		for( auto& otherPlayer : _players ) {
			DEBUG.LOG_ENTRY(std::stringstream() << "Sending player " << otherPlayer->GetName() << " to " << sharedPlayer->GetName());
			states.CommitTo( *otherPlayer, sharedPlayer );
		}

		_players.insert(shared);

#if 0
		for( const auto& object : objects ) {
			peer->Send(object);
		}
#endif
	};

	auto playerDisconnected = [&_players](Network::Peer& peer) {

		for( auto it = _players.cbegin(); it != _players.cend(); ++it ) {
			auto remotePlayerPtr = std::dynamic_pointer_cast<RemotePlayer>(*it);

			if( remotePlayerPtr == nullptr )
				continue; // Not a RemotePlayer

			if( *remotePlayerPtr == peer ) {
				_players.erase(it);
				break;
			}
		}
	};

	network->SetConnectCallback(playerConnected);
	network->SetDisconnectCallback(playerDisconnected);

	{ // Start Registration
		auto worker = std::bind(&hack::net::Registration::ExecuteWorker, std::ref(registration));
		futures.push_back(std::make_pair(std::async(ASYNC_POLICY, worker), &registration));
	}

	{ // Start State management
		auto worker = std::bind(&hack::state::States::ExecuteWorker, std::ref(states));
		futures.push_back(std::make_pair(std::async(ASYNC_POLICY, worker), &states));
	}

	if( needObjectCreation ) {
		// We need to create the basic objects

		auto stone = std::make_shared<Stone>();
		objects.Register( stone );

		//TODO: Validate against collision
		DEBUG.LOG_ENTRY(std::stringstream() << "FIRST");
	} else {
		DEBUG.LOG_ENTRY(std::stringstream() << "NO FIRST");
	}

	auto sharedAvatar = [&objects]() -> std::shared_ptr<Avatar> {
		// Create our player representation
		auto sharedAvatar = std::make_shared<Avatar>();
		sharedAvatar->setX( static_cast<int>(std::rand() / static_cast<float>(RAND_MAX) * WINDOW_WIDTH) );
		sharedAvatar->setY( static_cast<int>(std::rand() / static_cast<float>(RAND_MAX) * WINDOW_HEIGHT) );
		objects.Register( sharedAvatar );
		return sharedAvatar;
	}();

	{
		r->getInputmanager().registerCallbacks( getAvatarMoveHandler  ( sharedAvatar, objects ),
		                                        getMouseMoveHandler   ( sharedAvatar ),
		                                        getAvatarAttackHandler( sharedAvatar ) );

		r->run();
		// Runs till window is closed
		r = nullptr;
	}

	// Signal stop to all Subsystems
	for( auto& future : futures ) {
		future.second->StopWorker();
	}

	// Wait for Subsystems to terminate
	for( auto& future : futures ) {
		future.first.wait();
	}
}

