/*
 * states.cpp
 *
 *  Created on: Jun 23, 2012
 *      Author: andreas
 */

#include <sstream>
#include "states.hpp"

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

	if( _network )
		PassToNetwork( data );
	else
		_queue.push_back( data );
}

void States::CommitTo( const Serializable& object, hack::logic::Player& player) {
	std::stringstream stream;
	object.Serialize( stream );
	auto data = stream.str();

	//TODO: IMPLEMENT
}

//void CommitTo( const Serializable& object, Player& player);

void States::PassToNetwork( const std::string& data ) {
	_network->Send( data );
}

void States::SetNetwork( hack::net::Network& network ) {
	_network = &network;

	if( !_network )
		return;

	// We have a network connection
	for( auto& str : _queue ) {
		PassToNetwork( str );
	}

	// We passed everything to network so forget
	_queue.clear();
}


