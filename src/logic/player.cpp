
#include "player.hpp"

using namespace hack::logic;

Player::Player(std::string name) :
	_name(name)
{
}

Player::~Player() {
}

const std::string& Player::GetName() const {
	return _name;
}

std::ostream& Player::SerializeContent(const std::string& className, std::ostream& stream) const {
	return Serializable::SerializeContent( className, stream )
	<< ',' << GetUUID() << ',' << GetName();
}

bool Player::operator <(const Player& player) const {
	return GetUUID() < player.GetUUID();
}
