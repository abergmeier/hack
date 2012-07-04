/*
 * serializable.cpp
 *
 *  Created on: Jun 25, 2012
 *      Author: andreas
 */

#include "serializable.hpp"

using namespace hack::state;

void Serializable::Serialize( std::ostream& stream ) const {
	stream << '{';
	SerializeContent(stream);
	stream << '}';
}

std::ostream& Serializable::SerializeContent( std::ostream& stream ) const {
	return stream << GetClassName();
}

std::ostream& Serializable::String::Serialize( std::ostream& stream, const std::string& str ) {
	return stream << '{' << str.size() << ',' << str << '}';
}

std::string Serializable::String::Deserialize( std::istream& input ) {

	input.get(); // Skip {

	size_t size;
	input >> size;

	input.get(); // Skip ,
	std::string buffer;
	buffer.reserve( size );
	input.read( &buffer.front(), size );

	input.get(); // Skip }

	return buffer;
}

