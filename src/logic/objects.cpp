/*
 * objects.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#include <functional>
#include <iostream>
#include "objects.hpp"

using namespace hack::logic;

#ifndef _MSC_VER
typename 
#endif
	Objects::class_map_type Objects::CLASS_MAP;

namespace {
	bool intersect(vector2<float> d, vector2<float> f, float r) {
		float a = d.dot( d ) ;
		float b = 2*f.dot( d ) ;
		float c = f.dot( f ) - r*r ;

		float discriminant = b*b-4*a*c;
		if( discriminant < 0 ) {
			return false;
		    // no intersection
		}
			// ray didn't totally miss sphere,
			// so there is a solution to
			// the equation.

		discriminant = sqrt( discriminant );
		// either solution may be on or off the ray so need to test both
		float t1 = (-b + discriminant)/(2*a);
		float t2 = (-b - discriminant)/(2*a);
	
		if( t1 >= 0 && t1 <= 1 ) {
			return true;
			// t1 solution on is ON THE RAY.
		} 
		    // t1 solution "out of range" of ray

		if( t2 >= 0 && t2 <= 1 ) {
			return true;
			// t2 solution on is ON THE RAY.
		} else {
			return false;
			// t2 solution "out of range" of ray
		}
	}
}

Objects::value_type
Objects::Deserialize(std::istream& stream) {
	std::string className;
	stream >> className;
	id_type id(stream);

	auto it = _objectMap.find(id);

	Objects::value_type object;

	if( it == _objectMap.cend() ) {
		// Create new object
		auto classIt = CLASS_MAP.find(className);
		if( classIt == CLASS_MAP.cend() ) {
			std::cerr << "No class registration with name " << className << " found.";
			return nullptr;
		}

		auto& createFunc = (*classIt).second;
		object = createFunc(stream);
	} else {
		// Set already present object
		object = (*it).second;

		if( object ) {
			// Dead ugly cast, but everything else would require to mix entity
			// with serialization and that is even more ugly.
			auto objectPtr = std::dynamic_pointer_cast<Object>( object );
			if( objectPtr )
				*objectPtr = stream;
		}
	}

	return object;
}

std::pair<Objects::iterator, bool>
Objects::insert( const value_type& value ) {
	_insertHandlerCallback( value );
	auto result = _objectMap.insert( std::make_pair(value->getid(), value) );
	return std::make_pair( Objects::iterator(result.first), result.second );
}

Objects::size_type
Objects::erase( const key_type& key ) {
	_eraseHandlerCallback( key  );
	return _objectMap.erase( key->getid() );
}

/*
Objects::iterator
Objects::begin() {
	return Objects::iterator( _objectMap.begin() );
}

Objects::const_iterator
Objects::begin() const {
	return Objects::const_iterator( _objectMap.begin() );
}

Objects::const_iterator
Objects::cbegin() const {
	return Objects::const_iterator( _objectMap.cbegin() );
}

Objects::iterator
Objects::end() {
	return Objects::iterator( _objectMap.end() );
}

Objects::const_iterator
Objects::end() const {
	return Objects::const_iterator( _objectMap.end() );
}

Objects::const_iterator
Objects::cend() const {
	return Objects::const_iterator( _objectMap.cend() );
}
*/

Objects::iterator::iterator( object_map_type::iterator it ) :
	_it(it)
{
}

bool allintersect( ) {

}
