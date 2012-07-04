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

	template <typename T>
	struct vector2 : protected std::array<T, 2> {
		typedef std::array<T, 2> __BASE;
		typedef typename __BASE::reference reference;
		typedef typename __BASE::const_reference const_reference;
		typedef typename __BASE::size_type size_type;

		vector2() :
			__BASE()
		{
		}

		vector2(T first, T second) :
			__BASE()
		{
			(*this)[0] = first;
			(*this)[1] = second;
		}

		template <typename OT>
		T dot( const OT& other ) const {
			T result = 0;
			for( size_t i = 0; i != std::tuple_size<__BASE>::value; ++i ) {
				result += (*this)[i] * other[i];
			}
			return result;
		}

		reference operator[]( size_type pos ) {
			return __BASE::operator[]( pos );
		}
		const_reference operator[]( size_type pos ) const {
			return __BASE::operator[]( pos );
		}

		double length() const {
			double result = 0;
			for( size_t i = 0; i != std::tuple_size<__BASE>::value; ++i ) {
				result += std::pow( (*this)[i], 2 );
			}
			return std::sqrt( result );
		}
	};

	void UpdateRotation(vector2<int>& mousePosition, hack::logic::Avatar& playerAvatar) {

		static const vector2<int> ORIG_ROTATION(0, -1);
		static const auto         ORIG_LENGTH = ORIG_ROTATION.length();

		vector2<int> vector(
			mousePosition[0] - playerAvatar.getX(),
			mousePosition[1] - playerAvatar.getY()
		);

		auto cosrot = vector.dot( ORIG_ROTATION ) / ( vector.length() * ORIG_LENGTH );
		auto rot = std::acos( cosrot );

		//DEBUG.LOG_ENTRY(std::stringstream() << "Rotation: " << rot);

		playerAvatar.setAngle( rot );
	};
}

using namespace hack::logic;

int main() {

	std::set<std::shared_ptr<Player>> _players;

	RegisterAllClasses();

	auto sharedLocalPlayer = std::make_shared<hack::state::LocalPlayer>();
	_players.insert( sharedLocalPlayer );

	auto r = std::make_shared<renderer>(640, 480);
	Objects objects(r);

	hack::net::Network network;
	// Connect to all known Peers before we register ourselves
	// so we do not have to filter ourselves
	for( auto& other : hack::net::Registration::GetAll() ) {
		network.ConnectTo(other.host, other.port );
	}

	hack::net::Registration registration( sharedLocalPlayer->GetUUID(), network.GetIncomingPort() );

	auto& states = hack::state::States::Get();
	states.SetNetwork( network );

	auto playerConnected = [&_players, &states](std::shared_ptr<hack::net::Network::Peer> peer) {
		auto shared = std::make_shared<hack::net::RemotePlayer>(peer, "Unnamed");

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
		auto stone = std::make_shared<Stone>();
		auto sharedAvatar = std::make_shared<Avatar>();

		objects.Register( stone );
		objects.Register( sharedAvatar );

		vector2<int> lastMousePosition;

		auto mover = [&lastMousePosition, sharedAvatar]( int x, int y ) {
			sharedAvatar->setX( sharedAvatar->getX() + x );
			sharedAvatar->setY( sharedAvatar->getY() + y );

			DEBUG.LOG_ENTRY(std::stringstream() << "Avatar Pos: " << sharedAvatar->getX() << ':' << sharedAvatar->getY());

			UpdateRotation( lastMousePosition, *sharedAvatar );
		};

		auto mouseMoved = [&lastMousePosition, sharedAvatar]( int absx, int absy ) {

			// Save for further processing
			lastMousePosition[0] = absx;
			lastMousePosition[1] = absy;

			//DEBUG.LOG_ENTRY(std::stringstream() << "Mouse Pos: " << absx << ':' << absy);

			UpdateRotation( lastMousePosition, *sharedAvatar );
		};

		auto attacker = [sharedAvatar]() {
			//TODO: Implement attack
		};

		r->getInputmanager().registerCallbacks( mover,
		                                       mouseMoved,
		                                       attacker );

		r->run();
		r = nullptr;
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

