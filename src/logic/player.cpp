
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

std::ostream& Player::SerializeName( std::ostream& stream ) const {
	return String::Serialize( stream, GetName() );
}

std::ostream& Player::SerializeUUID( std::ostream& stream ) const {
	return String::Serialize( stream, GetUUID() );
}

std::ostream& Player::SerializeContent(std::ostream& stream) const {
	Serializable::SerializeContent( stream );
	stream << ',';
	SerializeUUID( stream );
	stream << ',';
	return SerializeName( stream );
}

bool Player::operator <(const Player& player) const {
	return GetUUID() < player.GetUUID();
}
