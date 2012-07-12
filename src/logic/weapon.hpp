#ifndef WEAPON_HPP_
#define WEAPON_HPP_

#include <istream>
#include <ostream>
#include <string>
#include "object.hpp"

namespace hack {
namespace logic {

class Weapon : public Object {
public:
	Weapon(std::string siteID) : Object(std::move(siteID)) {};
	Weapon(std::istream& stream);
	static const std::string NAME;
	const std::string& ClassName() const override;
};

} }

#endif /* WEAPON_HPP_ */
