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

const std::string&
LocalPlayer::GetClassName() const {
	return NAME;
}

LocalPlayer::LocalPlayer() :
	hack::logic::Player("Unnamed"),
	_uuid(UUID::Generate())
{
}

const std::string& LocalPlayer::GetUUID() const {
	return _uuid;
}

void LocalPlayer::Commit() {
	States::Get().Commit(*this);
	//TODO: Commit to be displayed
}

bool LocalPlayer::IsProcessLocal() const {
	return true;
}


