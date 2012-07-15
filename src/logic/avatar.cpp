#include <string>
#include <memory>
#include "avatar.hpp"

using namespace hack::logic;

const std::string Avatar::NAME("Avatar");

const std::string&
Avatar::ClassName() const {
	return NAME;
}

Avatar::Avatar(std::string siteID)
	: Object(std::move(siteID)), hitpoints(100), damage(10), radius(40)
{};


Avatar::Avatar(std::istream& stream) :
	Object(stream)
{
	SetNonConst(stream);
}

std::ostream& Avatar::SerializeContent(std::ostream& stream) const {
	return Object::SerializeContent( stream )
		<< ',' << hitpoints << ',' << damage << ',' << radius;
}

size_t Avatar::getHitpoints() const {
	return hitpoints;
}

size_t Avatar::getDamage() const {
	return damage;
}

float Avatar::getRadius() const {
	return radius;
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

void Avatar::SetNonConst(std::istream& stream) {
	 stream.get();
	 stream >> hitpoints;
	 stream.get();
	 stream >> damage;
	 stream.get();
	 stream >> radius;
}


Object& Avatar::operator = (std::istream& stream) {
	Object::operator = (stream);
	SetNonConst(stream);
	return *this;
}
