/*
 * local_player.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#include <functional>
#include <queue>
//#include <Poco/Base64Encoder.h>
#include <sstream>
#ifdef _MSC_VER
	#include <rpc.h>
#else
	#include <uuid/uuid.h>
#endif // DEBUG
#include "local_player.hpp"
#include "../include.hpp"
#include "../logic/states.hpp"


namespace {
	std::string generateUUID() {

		
#ifdef _MSC_VER
		UUID id;
		RPC_CSTR cstr;
		UuidCreate(&id);
		UuidToStringA(&id,&cstr);
		std::string uuid(reinterpret_cast<char*>(cstr));
		RpcStringFreeA(&cstr);
#else
		// We have to use array here because
		// std::string does not provide access to
		// internal char array
		std::array<char, 16> uuidBuffer;
		
		// Generate function wants buffer unsigned but string does only accept signed.
		// Since sizes are the same - cast to unsigned for function.
		uuid_generate_random(reinterpret_cast<unsigned char*>(uuidBuffer.data()));
		std::string uuid(uuidBuffer.data(), uuidBuffer.size());
#endif // _MSC_VER


		return std::move(uuid);
	}
}

using namespace hack::state;

const std::string LocalPlayer::NAME("LocalPlayer");

LocalPlayer::LocalPlayer() :
	hack::logic::Player("Unnamed"),
	_uuid(::generateUUID())
{
}

const std::string& LocalPlayer::GetUUID() const {
	return _uuid;
}

std::ostream& LocalPlayer::SerializeContent(std::ostream& stream) const {
	//Poco::Base64Encoder encoder(stream);
	return Player::SerializeContent( NAME, stream );
}

void LocalPlayer::Commit() {
	std::ostringstream stream;
	Serialize(stream);
	States::Get().Commit(stream.str());
	//TODO: Commit to be displayed
}


