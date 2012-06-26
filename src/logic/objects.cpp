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

Objects Objects::INSTANCE;

#ifndef _MSC_VER
typename 
#endif
	Objects::class_map_type Objects::CLASS_MAP;

Objects::Objects() :
	_objectMap(),
	_objects()
{
}

Objects& Objects::Get() {
	return INSTANCE;
}

std::shared_ptr<Object> Objects::Deserialize(std::istream& stream) {
	std::string className;
	stream >> className;
	id_type id(stream);

	auto it = Get()._objectMap.find(id);

	std::shared_ptr<Object> object;

	if( it == Get()._objectMap.cend() ) {
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

void Objects::Register(std::shared_ptr<Object> object) {
	_objectMap.insert( std::make_pair(object->getid(), object) );
	_objects.emplace_back( object );
}

Objects::iterator
Objects::begin() {
	return _objects.begin();
}

Objects::const_iterator
Objects::begin() const {
	return _objects.begin();
}

Objects::const_iterator
Objects::cbegin() const {
	return _objects.cbegin();
}

Objects::iterator
Objects::end() {
	return _objects.end();
}

Objects::const_iterator
Objects::end() const {
	return _objects.end();
}

Objects::const_iterator
Objects::cend() const {
	return _objects.cend();
}


