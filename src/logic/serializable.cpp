/*
 * serializable.cpp
 *
 *  Created on: Jun 25, 2012
 *      Author: andreas
 */

#include "serializable.hpp"

using namespace hack::state;

Serializable::Serializable() {
}

void Serializable::Serialize( std::ostream& stream ) const {
	stream << '{' << SerializeContent(stream) << '}';
}

std::ostream& Serializable::SerializeContent(const std::string& className, std::ostream& stream) const {
	return stream << className;
}


