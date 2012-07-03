#include <string>
#include <memory>
#include "avatar.hpp"

using namespace hack::logic;

const std::string Avatar::NAME("Avatar");

Avatar::Avatar(std::istream& stream) :
	Object(stream)
{
}

std::ostream& Avatar::SerializeContent(std::ostream& stream) const {
	return Object::SerializeContent(NAME, stream);
}

size_t Avatar::getHitpoints(){
	return hitpoints;
}

size_t Avatar::getDamage(){
	return damage;
}

void Avatar::setHitpoints(size_t value){
	hitpoints = value;
}

void Avatar::setDamage(size_t value) {
	damage = value;
}

void Avatar::hit(size_t value) {
	hitpoints = hitpoints-value;
}
