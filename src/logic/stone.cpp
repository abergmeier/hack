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

const std::string&
Stone::GetClassName() const {
	return NAME;
}

Stone::Stone(std::istream& stream) :
	Object(stream)
{
}

