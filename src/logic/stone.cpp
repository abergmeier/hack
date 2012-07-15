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
Stone::ClassName() const {
	return NAME;
}

Stone::Stone(std::string siteID) :
	Object( std::move(siteID) )
{
}

Stone::Stone(std::istream& stream) :
	Object(stream)
{
}

