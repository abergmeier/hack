/*
 * object.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#include "object.hpp"

using namespace hack::logic;

Object::Object(std::istream& stream) {
	stream >> id;
	Set(stream);
}

void Object::Set(std::istream& stream) {
	stream >> position;
}

Object& Object::operator = (std::istream& stream) {
	Set(stream);
	return *this;
}

void Object::Serialize(const std::string& className, std::ostream& stream) const {
	stream << className;
	stream << id;
	stream << position;
}


