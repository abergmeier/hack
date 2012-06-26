/*
 * stone.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#include <string>
#include <memory>
#include "stone.hpp"

using namespace hack::logic;

const std::string Stone::NAME("Stone");

Stone::Stone(std::istream& stream) :
	Object(stream)
{
}

std::ostream& Stone::SerializeContent(std::ostream& stream) const {
	return Object::SerializeContent(NAME, stream);
}

