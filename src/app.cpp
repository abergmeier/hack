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
		auto& localMousePosition = lastMousePosition;
		return [&localMousePosition, sharedAvatar, &obj]( int x, int y ) {

			int xx = sharedAvatar->getX() + x;
			int yy = sharedAvatar->getY() + y;

			if (obj.movementCheck(*sharedAvatar)) {
				sharedAvatar->setX(xx);
				sharedAvatar->setY(yy);
			}
			//DEBUG.LOG_ENTRY(std::stringstream() << "Avatar Pos: " << sharedAvatar->getX() << ':' << sharedAvatar->getY());

			UpdateRotation( localMousePosition, *sharedAvatar );
		};
	}

	std::function<void(int x, int y)> getMouseMoveHandler( std::shared_ptr<hack::logic::Avatar> sharedAvatar ) {
		auto& localMousePosition = lastMousePosition;
		return [&localMousePosition, sharedAvatar]( int absx, int absy ) {
			// Save for further processing
			localMousePosition[0] = absx;
			localMousePosition[1] = absy;

			//DEBUG.LOG_ENTRY(std::stringstream() << "Mouse Pos: " << absx << ':' << absy);

			UpdateRotation( localMousePosition, *sharedAvatar );
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

//fix for visual studio
#undef max
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

	auto others = Registration::GetAll();

	auto network = std::make_shared<hack::net::Network>( sharedLocalPlayer->GetUUID() );

	// Before doing anything else, we first have to register with the Server
	Registration registration( sharedLocalPlayer->GetUUID(), network->GetIPAddress(), network->GetIncomingPort() );

	// Setup states
	auto& states = States::Get();
	states.SetNetwork( network );
	states.SetDeserializer( [&objects]( std::istream& stream ) {
		objects.Deserialize( stream );
	});

	auto sharedAvatar = [&objects, &states]() -> std::shared_ptr<Avatar> {
		// Create our player representation
		auto sharedAvatar = std::make_shared<Avatar>();
		sharedAvatar->setX( static_cast<int>(std::rand() / static_cast<float>(RAND_MAX) * WINDOW_WIDTH) );
		sharedAvatar->setY( static_cast<int>(std::rand() / static_cast<float>(RAND_MAX) * WINDOW_HEIGHT) );
		objects.Register( sharedAvatar );
		states.Commit( *sharedAvatar );

		return sharedAvatar;
	}();

	bool hasData = false;
	auto createObjects = [&]() {
		hasData = true;
		others.clear();
		// We need to create the basic objects

		auto stone = std::make_shared<Stone>();
		objects.Register( stone );
		states.Commit( *stone );
	};

	auto playerConnected = [&](std::shared_ptr<hack::net::Network::Peer> peer) mutable {
		auto shared = std::make_shared<RemotePlayer>(peer, "Unnamed");

		for( auto it = others.begin(); it != others.end(); ++it ) {
			if( it->uuid == peer->uuid ) {
				others.erase( it );
				DEBUG.LOG_ENTRY(std::stringstream() << "PREVIOUS: " << others.size() );
				// Other peer has data
				hasData = true;
				// We no longer need others
				others.clear();
				break;
			}
		}

		if( others.empty() && !hasData ) {
			// No other peer before us
			// so we have to create the objects
			createObjects();
		}

		auto sharedPlayer = std::static_pointer_cast<Player>( shared );

		_players.insert(shared);

		for( const auto& object : objects ) {
			auto serializable = std::dynamic_pointer_cast<hack::state::Serializable>(object);
			states.CommitTo( *serializable, sharedPlayer );
		}
	};

	auto playerConnectedFailed = [&]( const std::string& ip, size_t port) mutable {
		for( auto it = others.begin(); it != others.end(); ++it ) {
			if( it->host == ip && it->port == port ) {
				others.erase( it );
				DEBUG.LOG_ENTRY(std::stringstream() << "PREVIOUS: " << others.size() );
				break;
			}
		}

		if( others.empty() && !hasData ) {
			// No other peer before us
			// so we have to create the objects
			createObjects();
		}
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
	network->SetConnectFailedCallback(playerConnectedFailed);
	network->SetDisconnectCallback(playerDisconnected);

	for( auto& other : others ) {
		network->ConnectTo( other.host, other.port, other.uuid );
	}

	r->getInputmanager().registerCallbacks( getAvatarMoveHandler  ( sharedAvatar, objects ),
											getMouseMoveHandler   ( sharedAvatar ),
											getAvatarAttackHandler( sharedAvatar ) );

	r->run();
	// Runs till window is closed
	r = nullptr;
}

