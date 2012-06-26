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
#include "local_player.hpp"
#include "../include.hpp"
#include "../logic/states.hpp"
#include "uuid.hpp"

using namespace hack::state;

const std::string LocalPlayer::NAME("LocalPlayer");

LocalPlayer::LocalPlayer() :
	hack::logic::Player("Unnamed"),
	_uuid(UUID::Generate())
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


