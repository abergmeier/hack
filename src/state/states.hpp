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
#include "../logic/player.hpp"

namespace hack {
namespace state {

class States : public hack::Subsystem {

	struct {
		std::mutex mutex;
		std::deque<std::string> queue;
	} _input;
	struct {
		typedef std::pair<std::weak_ptr<hack::logic::Player>, std::string> value_type;
		std::mutex mutex;
		std::deque<value_type> queue;
	} _output;
	bool _isRunning;
	std::function<void(std::istream&)> _deserialize;
	std::weak_ptr<hack::net::Network> _network;
	// If no network is connected save data locally

	bool PassToNetwork( const std::string& data );
	bool PassToNetwork( const std::string& data, hack::logic::Player& player );
	bool _ExecuteWorker();
	void ProcessInput();
	void ProcessOutput();
public:
	States( std::weak_ptr<hack::net::Network> network );
	~States();
	void Commit( const Serializable& object );
	void CommitTo( const Serializable& object, std::shared_ptr<hack::logic::Player> player);

	void SetDeserializer( std::function<void(std::istream&)> );
	void ReceiveFrom( std::string serialized, hack::logic::Player& player );

	void ExecuteWorker() override;
protected:
	void StopWorker() override;
};

} } //namespace hack


#endif /* STATES_HPP_ */
