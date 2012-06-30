/*
 * uuid.cpp
 *
 *  Created on: Jun 26, 2012
 *      Author: andreas
 */

#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>
#include "uuid.hpp"

using namespace hack;

std::string hack::UUID::Generate() {
	Poco::UUIDGenerator& generator = Poco::UUIDGenerator::defaultGenerator();
	Poco::UUID uuid(generator.createRandom());
	return uuid.toString();
}


