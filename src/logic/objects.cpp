/*
 * objects.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#include <functional>
#include <iostream>
#include <memory>
#include "objects.hpp"
#include "stone.hpp"
#include "../debug.hpp"
#ifdef _MSC_VER
#define _USE_MATH_DEFINES // for C++
#include <math.h>
#endif // _MSC_VER

using namespace hack::logic;

#ifndef _MSC_VER
typename 
#endif
	Objects::class_map_type Objects::CLASS_MAP;

namespace {

	struct Debug : public hack::Debug {
		const std::string& GetCategory() const override {
			static const std::string CATEGORY = "Objects";
			return CATEGORY;
		}
	};

	const Debug DEBUG;

	//intersection test of a line with a circle
	//start and end are the to end points of the like, pos is the position if the circle and r is the radius
	bool intersect(vector2<float> &start, vector2<float> &end, vector2<float> &pos, float r) {
		//direction vector of the line
		vector2<float> d = start.sub(end);
		
		//direction vector of the circle, starting the one point of the line
		vector2<float> f(pos[0]-start[0],pos[1]-start[1]);
		
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
			// t1 solution on is ON THE RAY.
			return true;
		}
		if( t2 >= 0 && t2 <= 1 ) {
			return true;
			// t2 solution on is ON THE RAY.
		}
		return false;	
	}

	//rotates a vector around a specific position and return a vector2
	vector2<float> rotateVector(float x, float y, int px, int py, float angle) {
		angle = static_cast<float>(angle * M_PI / 180);
		float dx = x - static_cast<float>(px);
		float dy = y - static_cast<float>(py);
		x = (dx * std::cos(angle) + dy * -std::sin(angle)) + static_cast<float>(px);
		y = (dx * std::sin(angle) + dy * std::cos(angle)) + static_cast<float>(py);

		return vector2<float>(x,y);
	}

	//intersection test of a circle with all four lines of a square
	bool intersectAll(const entity& obj, const Avatar& avatar, const vector2<int>& possibleChange) {
		//static object intersection
		//calculating corners of the object
		float x1 = obj.getX() - static_cast<float>(obj.getWidth()) / 2;
		float y1 = obj.getY() - static_cast<float>(obj.getHeight()) / 2;
		float x2 = obj.getX() + static_cast<float>(obj.getWidth()) / 2;
		float y2 = obj.getY() - static_cast<float>(obj.getHeight()) / 2;
		float x3 = obj.getX() + static_cast<float>(obj.getWidth()) / 2;
		float y3 = obj.getY() + static_cast<float>(obj.getHeight()) / 2;
		float x4 = obj.getX() - static_cast<float>(obj.getWidth()) / 2;
		float y4 = obj.getY() + static_cast<float>(obj.getHeight()) / 2;

		//rotate the corners to their original place
		vector2<float> A = rotateVector(x1,y1,obj.getX(),obj.getY(),obj.getAngle());
		vector2<float> B = rotateVector(x2,y2,obj.getX(),obj.getY(),obj.getAngle());
		vector2<float> C = rotateVector(x3,y3,obj.getX(),obj.getY(),obj.getAngle());
		vector2<float> D = rotateVector(x4,y4,obj.getX(),obj.getY(),obj.getAngle());
		vector2<float> pos(static_cast<float>(possibleChange[0]),static_cast<float>(possibleChange[1]));

		//check if lines between the corners intersect with the player
		if(intersect(A,B,pos,avatar.getRadius()) 
			|| intersect(B,C,pos,avatar.getRadius())
			|| intersect(C,D,pos,avatar.getRadius())
			|| intersect(D,A,pos,avatar.getRadius())) {
			return true;
		}
		return false;
	}
}

Objects::value_type
Objects::Deserialize(std::istream& stream) {
	char extracted = stream.get(); // Skip {
	auto className = hack::state::Serializable::String::Deserialize( stream );

	extracted = stream.get(); // Skip ,

	object_map_type::iterator it;
	{
		// Remember position of id - we might
		// need to jump back to it
		const auto streamPodId = stream.tellg();

		id_type id(stream);

		// See whether we already have the id
		it = _objectMap.find(id);

		// We need to be able to extract id, too
		// so rewind stream back to id
		stream.seekg( streamPodId );
	}

	Objects::value_type object;

	if( it == _objectMap.cend() ) {
		// Create new object
		auto classIt = CLASS_MAP.find(className);
		if( classIt == CLASS_MAP.cend() ) {
			DEBUG.ERR_ENTRY( std::string("No class registration with name ") + className + " found." );
			return nullptr;
		}

		auto& createFunc = (*classIt).second;
		object = createFunc(stream);
		Register( object );
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

	extracted = stream.get(); //Ignore }
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

Objects::iterator::iterator( object_map_type::iterator it ) :
	_it(it)
{
}

Objects::const_iterator::const_iterator( object_map_type::const_iterator it ) :
	_it(it)
{

}

bool Objects::movementCheck(const hack::logic::Avatar &avatar, const vector2<int>& possibleChange)  {
	for(auto &e : _objectMap) {
		//checking if its not yourself
		if (*e.second == avatar) 
			continue;

		//world collision	
		if(e.second->ClassName() == hack::logic::Stone::NAME) {
			if(intersectAll(*e.second,avatar,possibleChange))
				return false;
		//player collsision
		} else if (e.second->ClassName() == hack::logic::Avatar::NAME) {
			float dx = static_cast<float>(possibleChange[0] - e.second->getX());
			float dy = static_cast<float>(possibleChange[1] - e.second->getY());
			float length = std::sqrt(dy * dy + dx * dx);
	
			return length > avatar.getRadius()*2;
		}
	}
	//nothing in the way, can move on
	return true;
}

//dont use this function like this
//it contains unfinished implementation and pseudo code
bool Objects::attackCheck(const hack::logic::Weapon &weapon, const hack::logic::Avatar &avatar) {
	//vectorOfHitAvatars hitAvatars;
	for(auto &e : _objectMap){ 
		if(e.second->ClassName() == hack::logic::Avatar::NAME) {
			if(intersectAll(weapon,dynamic_cast<Avatar&>(*e.second),vector2<int>(e.second->getX(),e.second->getY())))
				/*hitAvatars.push_back(avatar);*/; //another avatar got hit
		}
	}
	//return hitAvatars;
	return false;
}
