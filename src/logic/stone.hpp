/*
 * stone.hpp
 *
 *  Created on: Jun 23, 2012
 *      Author: andreas
 */

#ifndef STONE_HPP_
#define STONE_HPP_

#include <istream>
#include <ostream>
#include <string>
#include "object.hpp"

namespace hack {
namespace logic {

class Stone : public Object {
public:
	Stone() : Object() {};
	Stone(std::istream& stream);
	static const std::string NAME;
	const std::string& ClassName() const override;
};

} }

#endif /* STONE_HPP_ */
