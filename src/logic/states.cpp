/*
 * states.cpp
 *
 *  Created on: Jun 23, 2012
 *      Author: andreas
 */

#include <sstream>
#include "states.hpp"
#include "../net/remote_player.hpp"

using namespace hack::state;

States States::INSTANCE;

States& States::Get() {
	return INSTANCE;
}

States::States() :
	_network(nullptr)
{
}

void States::Commit( const Serializable& object ) {
	std::stringstream stream;
	object.Serialize( stream );
	auto data = stream.str();

	if( !PassToNetwork( data ) ) {
		std::weak_ptr<hack::logic::Player> weakPlayer;
		_queue.push_back( std::make_pair( weakPlayer, data ) );
	}
}

void States::CommitTo( const Serializable& object, std::shared_ptr<hack::logic::Player> player) {

	if( !player )
		return;

	if( player->IsProcessLocal() )
		return;

	// Only send if it Player is remote

	std::stringstream stream;
	object.Serialize( stream );
	auto data = stream.str();

	if( !PassToNetwork( data, *player ) ) {
		auto weakPlayer = std::weak_ptr<hack::logic::Player>( player );
		_queue.push_back( std::make_pair( weakPlayer, data ) );
	}
}

//void CommitTo( const Serializable& object, Player& player);

bool States::PassToNetwork( const std::string& data ) {
	if( !_network )
		return false;

	_network->Send( data );
	return true;
}

bool States::PassToNetwork( const std::string& data, hack::logic::Player& player ) {
	if( !_network )
		return false; // Cannot use network

	auto ptr = dynamic_cast<hack::net::RemotePlayer*>( &player );
	if( !ptr )
		return true; // Seems like player is no longer present

	ptr->SendTo( data );
	return true;
}

void States::SetNetwork( hack::net::Network& network ) {
	_network = &network;

	if( !_network )
		return;

	// We have a network connection
	for( auto& str : _queue ) {
		auto sharedPlayer = str.first.lock();

		auto remotePlayer = std::dynamic_pointer_cast<hack::net::RemotePlayer>( sharedPlayer );

		if( remotePlayer ) {
			remotePlayer->SendTo( str.second );
		} else {
			PassToNetwork( str.second );
		}
	}

	// We passed everything to network so forget
	_queue.clear();
}


