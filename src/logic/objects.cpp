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
	Objects::object_map_type Objects::OBJECTS;
#ifndef _MSC_VER
typename 
#endif
	Objects::class_map_type Objects::CLASS_MAP;

Object::~Object() {
}

std::shared_ptr<Object> Objects::Deserialize(std::istream& stream) {
	std::string className;
	stream >> className;
	id_type id(stream);

	auto it = OBJECTS.find(id);

	std::shared_ptr<Object> object;

	if( it == OBJECTS.cend() ) {
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
		object = (*it).second.lock();

		if( object )
			*object = stream;
	}

	return object;
}




