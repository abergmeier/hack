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
			static const std::string CATEGORY = "State management";
			return CATEGORY;
		}
	};

	const Debug DEBUG;
}

States States::INSTANCE;

States& States::Get() {
	return INSTANCE;
}

States::States() :
	_network( std::shared_ptr<hack::net::Network>() )
{
}

States::~States() {
	StopWorker();

	// Wait until it is save to destruct
	std::lock_guard<std::mutex> lock( destructorMutex );
}

void States::Commit( const Serializable& object ) {

	std::stringstream stream;
	object.Serialize( stream );

	std::weak_ptr<hack::logic::Player> weakPlayer;
	_output.queue.push_back( std::make_pair( weakPlayer, stream.str() ) );
}

void States::CommitTo( const Serializable& object, std::shared_ptr<hack::logic::Player> player) {

	if( !player )
		return;

	if( player->IsProcessLocal() )
		return;

	// Only send if it Player is remote

	std::stringstream stream;
	object.Serialize( stream );

	auto weakPlayer = std::weak_ptr<hack::logic::Player>( player );
	_output.queue.push_back( std::make_pair( weakPlayer, stream.str() ) );
}

//void CommitTo( const Serializable& object, Player& player);

bool States::PassToNetwork( const std::string& data ) {
	auto sharedNetwork = _network.lock();

	if( !sharedNetwork )
		return false;

	sharedNetwork->Send( data );
	return true;
}

bool States::PassToNetwork( const std::string& data, hack::logic::Player& player ) {

	auto ptr = dynamic_cast<hack::net::RemotePlayer*>( &player );
	if( !ptr )
		return true; // Player not remote

	ptr->SendTo( data );
	return true;
}

void States::SetNetwork( std::weak_ptr<hack::net::Network> network ) {
	_network = network;
}

void States::ReceiveFrom( std::string&& serialized, hack::logic::Player& player ) {
	std::lock_guard<std::mutex> lock( _input.mutex );
	// Push to the front queue
	_input.queue.push_back( serialized );
}

void States::ProcessInput() {
	{
		std::lock_guard<std::mutex> lock( _input.mutex );
		// Save elements in the backup queue so that the
		// front queue is accessible again
		_input.queue.swap( _input.backup_queue );
	}

	// Now we can safely work on the backup queue
	for( auto& element : _input.backup_queue ) {
		std::istringstream stream( element );
		_deserialize( stream );
	}

	_input.backup_queue.clear();
}

void States::ProcessOutput() {
	{
		std::lock_guard<std::mutex> lock( _output.mutex );
		// Save elements in the backup queue so that the
		// front queue is accessible again
		_output.queue.swap( _output.backup_queue );
	}

	// Now we can safely work on the backup queue
	for( auto& element : _output.backup_queue ) {
		auto sharedPlayer = element.first.lock();
		if( sharedPlayer )
			PassToNetwork( element.second, *sharedPlayer );
		else
			PassToNetwork( element.second );
	}

	_output.backup_queue.clear();
}

bool States::_ExecuteWorker() {

	ProcessInput();
	ProcessOutput();

	return _isRunning;
}

void States::ExecuteWorker() {

	static const std::chrono::milliseconds SLEEP_DURATION( 5 );

	DEBUG.LOG_ENTRY( "[Worker] Start..." );

	_isRunning = true;

	// Make sure nobody destructs object as long as this
	// function is running
	std::lock_guard<std::mutex> lock( destructorMutex );

	while( _ExecuteWorker() ) {

		if( _input.queue.empty() )
			// If there is nothing to do - do not spam
			// the CPU
			std::this_thread::sleep_for( SLEEP_DURATION );
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


