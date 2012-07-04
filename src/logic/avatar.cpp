#include <string>
#include <memory>
#include "avatar.hpp"

using namespace hack::logic;

const std::string Avatar::NAME("Avatar");

const std::string&
Avatar::GetClassName() const {
	return NAME;
}

Avatar::Avatar() : Object() {};


Avatar::Avatar(std::istream& stream) :
	Object(stream)
{
	Set(stream);
}

std::ostream& Avatar::SerializeContent(std::ostream& stream) const {
	return Object::SerializeContent( stream )
		<< ',' << hitpoints << ',' << damage ;
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
	hitpoints -= std::min(hitpoints, value);
}

void Avatar::Set(std::istream& stream) {
	 stream.get();
	 stream >> hitpoints;
	 stream.get();
	 stream >> damage;
}


Object& Avatar::operator = (std::istream& stream) {
	Object::operator = (stream);
	Set(stream);
	return *this;
}
