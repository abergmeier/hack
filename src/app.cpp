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
#include "state/states.hpp"
#include "graphics/renderer.hpp"
#include "debug.hpp"
#include "net/registration.hpp"
#include "logic/avatar.hpp"
#include "logic/weapon.hpp"

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

	// Register classes so that they may later on be
	// deserialized
	void RegisterAllClasses() {
		hack::logic::Objects::Register<hack::logic::Stone>();
		hack::logic::Objects::Register<hack::logic::Avatar>();
		hack::logic::Objects::Register<hack::logic::Weapon>();
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

		playerAvatar.setAngle( static_cast<float>(rot) );
	};

	float attack = 0;

	void updateWeaponPosition(hack::logic::Avatar& avatar, hack::logic::Weapon& weapon) {
		double angle = (avatar.getAngle()) * M_PI / 180;
		double rad = avatar.getRadius();
		int xx = static_cast<int>(rad * std::cos(angle) - attack * rad * -std::sin(angle));
		int yy = static_cast<int>(rad * std::sin(angle) - attack * rad * std::cos(angle));
		weapon.setX(xx+avatar.getX());
		weapon.setY(yy+avatar.getY());
	}

	const int WINDOW_WIDTH = 640;
	const int WINDOW_HEIGHT = 480;

	vector2<int> lastMousePosition;

	std::function<void(int x, int y)> getAvatarMoveHandler( std::shared_ptr<hack::logic::Avatar> sharedAvatar, std::shared_ptr<hack::logic::Weapon> sharedWeapon,  hack::logic::Objects &obj, hack::state::States& states ) {
		auto& localMousePosition = lastMousePosition;
		return [&localMousePosition, sharedAvatar, sharedWeapon, &states, &obj]( int x, int y ) {

			int xx = sharedAvatar->getX() + x;
			int yy = sharedAvatar->getY() + y;
			vector2<int> possibleChange(xx,yy);

			if (obj.movementCheck(*sharedAvatar,possibleChange)) {
				//player position
				sharedAvatar->setX(xx);
				sharedAvatar->setY(yy);
				
				//sword position in relation to player
				updateWeaponPosition(*sharedAvatar,*sharedWeapon);
			}
			//DEBUG.LOG_ENTRY(std::stringstream() << "Avatar Pos: " << sharedAvatar->getX() << ':' << sharedAvatar->getY());

			//update rotations
			UpdateRotation( localMousePosition, *sharedAvatar );
			states.Commit( *sharedAvatar );

			sharedWeapon->setAngle( sharedAvatar->getAngle() );
			states.Commit( *sharedWeapon );
		};
	}

	std::function<void(int x, int y)> getMouseMoveHandler( std::shared_ptr<hack::logic::Avatar> sharedAvatar, std::shared_ptr<hack::logic::Weapon> sharedWeapon,hack::logic::Objects &obj, hack::state::States& states) {
		auto& localMousePosition = lastMousePosition;
		return [&localMousePosition, sharedAvatar, sharedWeapon, &obj, &states]( int absx, int absy ) {
			// Save for further processing
			localMousePosition[0] = absx;
			localMousePosition[1] = absy;

			//DEBUG.LOG_ENTRY(std::stringstream() << "Mouse Pos: " << absx << ':' << absy);

			UpdateRotation( localMousePosition, *sharedAvatar );
			states.Commit( *sharedAvatar );

			//sword position in relation to player
			updateWeaponPosition(*sharedAvatar, *sharedWeapon);

			//sword angle
			sharedWeapon->setAngle(sharedAvatar->getAngle());
			states.Commit( *sharedWeapon );
		};
	}

	std::function<void(bool)> getAvatarAttackHandler( std::shared_ptr<hack::logic::Avatar> sharedAvatar,  std::shared_ptr<hack::logic::Weapon> sharedWeapon, hack::logic::Objects &obj, hack::state::States& states ) {
		return [sharedAvatar,sharedWeapon, &obj, &states](bool attacking) {
			if (attacking) {
				attack = 1;
				updateWeaponPosition(*sharedAvatar, *sharedWeapon);
				//pseudo code:
				/*vectorOfHitAvatars hitAvatars = obj.attackCheck(*sharedWeapon,*sharedAvatar);
				if(hitAvatars.size() > 0)
					processDamage(hitAvatars);*/
			} else {
				attack = 0;
				updateWeaponPosition(*sharedAvatar, *sharedWeapon);
			}
			
			states.Commit( *sharedWeapon );
		};
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

	auto network = std::make_shared<hack::net::Network>( sharedLocalPlayer->GetUUID(), others );

	// Before doing anything else, we first have to register with the Server
	Registration registration( sharedLocalPlayer->GetUUID(), network->GetIPAddress(), network->GetIncomingPort() );

	// Setup states
	auto states = std::make_shared<States>( network );
	states->SetDeserializer( [&objects]( std::istream& stream ) {
		objects.Deserialize( stream );
	});

	//weapon object for displaying and collision detection
	auto sharedAvatar = [&]() -> std::shared_ptr<Avatar> {
		// Create our player representation
		auto sharedAvatar = std::make_shared<Avatar>( sharedLocalPlayer->GetUUID() );
		std::srand( time(0) );
		sharedAvatar->setX( static_cast<int>(std::rand() / static_cast<float>(RAND_MAX) * WINDOW_WIDTH) );
		sharedAvatar->setY( static_cast<int>(std::rand() / static_cast<float>(RAND_MAX) * WINDOW_HEIGHT) );
		objects.Register( sharedAvatar );
		states->Commit( *sharedAvatar );

		return sharedAvatar;
	}();

	//weapon object for displaying and collision detection
	auto sharedWeapon = [&]() -> std::shared_ptr<Weapon> {
		// Create our weapon representation
		auto sharedWeapon = std::make_shared<Weapon>( sharedLocalPlayer->GetUUID() );
		//setting its initial position next to the character
		sharedWeapon->setX(static_cast<int>(sharedAvatar->getX())+static_cast<int>(sharedAvatar->getRadius()));
		sharedWeapon->setY(static_cast<int>(sharedAvatar->getY()));
		//setting its size
		sharedWeapon->setWidth(26);
		sharedWeapon->setHeight(74);
		objects.Register( sharedWeapon );
		states->Commit( *sharedWeapon );

		return sharedWeapon;
	}();

	bool hasData = false;
	auto attemptCreateObjects = [&]() {

		if( !others.empty() || hasData )
			return;

		// No other peer before us
		// so we have to create the objects

		hasData = true;
		others.clear();
		// We need to create the basic objects

		auto stone = std::make_shared<Stone>( sharedLocalPlayer->GetUUID() );
		stone->setX(200);
		stone->setY(200);
		stone->setAngle(45);
		objects.Register( stone );
		states->Commit( *stone );
	};

	auto playerConnected = [&](std::shared_ptr<hack::net::Network::Peer> peer) mutable {

		for( auto it = others.begin(); it != others.end(); ++it ) {
			if( it->uuid == peer->uuid ) {
				others.erase( it );
				// Other peer has data
				hasData = true;
				// We no longer need others
				others.clear();
				break;
			}
		}

		attemptCreateObjects();

		auto shared = std::make_shared<RemotePlayer>( network, states, peer, "Unnamed");
		_players.insert(shared);

		for( const auto& object : objects ) {
			auto serializable = std::dynamic_pointer_cast<hack::state::Serializable>(object);
			states->CommitTo( *serializable, shared );
		}
	};

	auto playerConnectedFailed = [&]( const std::string& ip, size_t port) mutable {
		for( auto it = others.begin(); it != others.end(); ++it ) {
			if( it->host == ip && it->port == port ) {
				others.erase( it );
				break;
			}
		}

		attemptCreateObjects();
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

	// Connect to all already registered hosts
	for( auto it = others.cbegin(); it != others.cend(); ) {
		if( network->ConnectTo( it->host, it->port, it->uuid ) )
			++it;
		else {
			// Save current iterator which
			// will be invalidated
			auto current = it;
			++it;
			others.erase( current );
			// current is invalidated, which
			// does not matter, since it was copied
			// and is no longer used
		}
	}

	attemptCreateObjects();

	r->getInputmanager().registerCallbacks( getAvatarMoveHandler  ( sharedAvatar, sharedWeapon , objects, *states ),
											getMouseMoveHandler   ( sharedAvatar, sharedWeapon , objects, *states ),
											getAvatarAttackHandler( sharedAvatar, sharedWeapon , objects, *states ) );

	r->run();
	// Runs till window is closed

	// Make sure callbacks are cleaned up
	r->getInputmanager().registerCallbacks( nullptr,
	                                        nullptr,
	                                        nullptr );
	r = nullptr;
}

