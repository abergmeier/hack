#include <string>
#include <memory>
#include "weapon.hpp"

using namespace hack::logic;

const std::string Weapon::NAME("Weapon");

const std::string&
Weapon::ClassName() const {
	return NAME;
}

Weapon::Weapon(std::string siteID) :
	Object( std::move(siteID) )
{
}

Weapon::Weapon(std::istream& stream) :
	Object( stream )
{
}

