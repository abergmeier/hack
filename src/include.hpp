/*
 * include.hpp
 *
 *  Created on: Jun 22, 2012
 *      Author: andreas
 */

#ifndef _INCLUDE_HPP_
#define _INCLUDE_HPP_

#include <array>
#include <ostream>
#include <istream>

template <class T>
std::ostream& _left_shift_operator(std::ostream& stream, const T& array) {
	stream << '[';
	for( auto it = array.cbegin(); it != array.cend(); ) {

		stream << *it;
		++it;
		if( it != array.cend() ) {
			stream << ',';
		}
	}
	stream << ']';
	return stream;
}

template <class T>
std::istream& _right_shift_operator(std::istream& stream, T& outputArray) {
	T array;
	char terminator;
	stream.get(terminator); // Should be '['

	for( auto it = array.begin(); it != array.end(); ) {

		stream >> *it;
		++it;
		if( it != array.cend() ) {
			stream.get(terminator); // Should be ','
		}
	}
	stream.get(terminator); // Should be ']'
	outputArray = std::move(array);
	return stream;
}

#endif // _INCLUDE_HPP_
