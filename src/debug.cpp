/*
 * debug.cpp
 *
 *  Created on: Jun 28, 2012
 *      Author: andreas
 */

#include <iostream>
#include <ostream>
#include <string>
#include "debug.hpp"

using namespace hack;

std::ostream& Debug::LOG( std::ostream& stream ) const {
	return std::cout << '[' << GetCategory() << "] " << stream.rdbuf();
}

std::ostream& Debug::LOG( const std::string& str ) const {
	return std::cout << '[' << GetCategory() << "] " << str;
}

std::ostream& Debug::ERR( std::ostream& stream ) const {
	return std::cerr << '[' << GetCategory() << "] " << stream.rdbuf();
}

std::ostream& Debug::ERR( const std::string& str ) const {
	return std::cerr << '[' << GetCategory() << "] " << str;
}

void Debug::LOG_ENTRY( std::ostream& stream ) const {
	 LOG( stream ) << std::endl;
}

void Debug::LOG_ENTRY( const std::string& str ) const {
	 LOG( str ) << std::endl;
}

void Debug::ERR_ENTRY( std::ostream& stream ) const {
	 ERR( stream ) << std::endl;
}

void Debug::ERR_ENTRY( const std::string& str ) const {
	 ERR( str ) << std::endl;
}
