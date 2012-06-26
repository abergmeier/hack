/*
 * states.hpp
 *
 *  Created on: Jun 12, 2012
 *      Author: andreas
 */

#ifndef STATES_HPP_
#define STATES_HPP_

#include <map>
#include <memory>
#include <deque>
#include <string>
#include "../net/network.hpp"
#include "serializable.hpp"
#include "player.hpp"

namespace hack {
namespace state {

class States {
	static States INSTANCE;
	hack::net::Network* _network;
	// If no network is connected save data locally
	std::deque<std::string> _queue;
	void PassToNetwork( const std::string& data );
	States();
public:
	static States& Get();
	void Commit( const Serializable& object );
	void CommitTo( const Serializable& object, Player& player);
	//std::map< std::weak_ptr<void>, std::unique_ptr<void> > _state;
	void SetNetwork( hack::net::Network& network );
};

} } //namespace hack


#endif /* STATES_HPP_ */
