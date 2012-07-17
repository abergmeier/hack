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

namespace {
	struct Debug : public hack::Debug {
		const std::string& GetCategory() const override {
			static const std::string CATEGORY = "States";
			return CATEGORY;
		}
	};

	const Debug DEBUG;
}

States::States( std::weak_ptr<hack::net::Network> network) :
	_network( network )
{
	worker = [&]() {
		auto worker = std::bind(&hack::state::States::ExecuteWorker, std::ref(*this));
		return std::async(ASYNC_POLICY, worker);
	}();
}

States::~States() {
	SaveStopWorker();
}

void States::Commit( const Serializable& object ) {

	std::ostringstream stream;
	object.Serialize( stream );
	stream.flush();

	std::weak_ptr<hack::logic::Player> weakPlayer;
	std::lock_guard<std::mutex> lock( _output.mutex );
	_output.queue.push_back( std::make_pair( weakPlayer, stream.str() ) );
}

void States::CommitTo( const Serializable& object, std::shared_ptr<hack::logic::Player> player) {

	if( !player )
		return;

	if( player->IsProcessLocal() )
		return;

	// Only send if it Player is remote

	std::ostringstream stream;
	object.Serialize( stream );
	stream.flush();

	std::lock_guard<std::mutex> lock( _output.mutex );
	_output.queue.push_back( std::make_pair(  player , stream.str() ) );
}

//void CommitTo( const Serializable& object, Player& player);

bool States::PassToNetwork( const std::string& data ) {
	auto sharedNetwork = _network.lock();

	if( !sharedNetwork )
		return false;

	sharedNetwork->SendTo( data );
	return true;
}

bool States::PassToNetwork( const std::string& data, hack::logic::Player& player ) {

	auto ptr = dynamic_cast<hack::net::RemotePlayer*>( &player );
	if( !ptr )
		return true; // Player not remote

	ptr->SendTo( data );
	return true;
}

void States::ReceiveFrom( std::string&& serialized, hack::logic::Player& player ) {
	std::lock_guard<std::mutex> lock( _input.mutex );
	// Push to the front queue
	_input.queue.push_back( serialized );
}

void States::ProcessInput() {
	decltype(_input.queue) input_queue;

	{
		std::lock_guard<std::mutex> lock( _input.mutex );

		if( _input.queue.empty() )
			return;

		// Save elements in the backup queue so that the
		// front queue is accessible again
		_input.queue.swap( input_queue );
	}

	// Now we can safely work on the backup queue
	for( auto& element : input_queue ) {
		std::istringstream stream( element );
		_deserialize( stream );
	}
}

void States::ProcessOutput() {
	decltype(_output.queue) output_queue;

	{
		std::lock_guard<std::mutex> lock( _output.mutex );

		if( _output.queue.empty() )
			return;

		// Save elements in the backup queue so that the
		// front queue is accessible again
		_output.queue.swap( output_queue );
	}

	// Now we can safely work on the backup queue
	for( auto& element : output_queue ) {
		auto sharedPlayer = element.first.lock();
		if( sharedPlayer )
			PassToNetwork( element.second, *sharedPlayer );
		else
			PassToNetwork( element.second );
	}
}

bool States::_ExecuteWorker() {

	ProcessInput();
	ProcessOutput();

	return _isRunning;
}

void States::ExecuteWorker() {

	DEBUG.LOG_ENTRY( "[Worker] Start..." );

	_isRunning = true;

	while( _ExecuteWorker() ) {

		// Value would simply be input.empty() && output.empty()
		// but sadly we have to handle mutexes
		bool queueEmpty;

		{
			std::lock_guard<std::mutex> lock( _input.mutex );
			queueEmpty = _input.queue.empty();
		}

		if( queueEmpty ) {
			std::lock_guard<std::mutex> lock( _output.mutex );
			queueEmpty = _output.queue.empty();
		}

		if( queueEmpty ) {
			// If there is nothing to do - do not spam
			// the CPU
			static const std::chrono::milliseconds SLEEP_DURATION( 5 );
			std::this_thread::sleep_for( SLEEP_DURATION );
		}
	}

	DEBUG.LOG_ENTRY( "[Worker] ...Stop" );
}

void States::StopWorker() {
	DEBUG.LOG_ENTRY("[Worker] Stopping...");
	_isRunning = false;
}

void States::SetDeserializer( std::function<void(std::istream&)> desFunc ) {
	_deserialize = desFunc;
}


