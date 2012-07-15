/*
 * serializable.cpp
 *
 *  Created on: Jun 25, 2012
 *      Author: andreas
 */

#include <stdexcept>
#include <string.h> //for strnlen
#include "serializable.hpp"

using namespace hack::state;

void Serializable::Serialize( std::ostream& stream ) const {
	stream << '{';
	SerializeContent(stream);
	stream << '}';
}

std::ostream& Serializable::SerializeContent( std::ostream& stream ) const {
	return String::Serialize( stream, ClassName() );
}

std::ostream& Serializable::String::Serialize( std::ostream& stream, const std::string& str ) {
	return stream << '{' << str.size() << ',' << str << '}';
}

std::string Serializable::String::Deserialize( std::istream& input ) {

	char extracted = input.get(); // Skip {

	size_t size;
	input >> size;

	extracted = input.get(); // Skip ,
	std::string buffer(size, '\0');
	input.read( &buffer.front(), buffer.length() );

	buffer.resize( strnlen(buffer.data(), size) );
	extracted = input.get(); // Skip }

	if( input.bad() )
		throw std::runtime_error("Invalid format");

	return buffer;
}

