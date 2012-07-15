/*
 * object.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#include "object.hpp"
#include "../include.hpp"

namespace {
	size_t NEXT_ID = 1;
}

using namespace hack::logic;

Object::Object(std::string siteID) :
	entity(),
	hack::state::Serializable(),
	id(std::move(siteID), NEXT_ID),
	x(0), y(0), angle(0), width(100), height(100)
{
	++NEXT_ID;
}

Object::Object(std::istream& stream) :
	id(stream)
{
	SetNonConst(stream);
}

Object::~Object() {
}

void Object::SetNonConst(std::istream& stream) {
	char extracted = stream.get(); // Skip ,
	stream >> x;
	extracted = stream.get(); // Skip ,
	stream >> y;
	extracted = stream.get(); // Skip ,
	stream >> width;
	extracted = stream.get(); // Skip ,
	stream >> height;
	extracted = stream.get(); // Skip ,
	stream >> angle;
}

Object& Object::operator = (std::istream& stream) {
	id_type tempId(stream); // Skip id

	if( tempId == id )
		throw std::runtime_error("Deserialize while ID does not match!");

	SetNonConst(stream);
	return *this;
}

std::ostream& Object::SerializeContent(std::ostream& stream) const {
	return Serializable::SerializeContent( stream )
		<< ',' << id << ',' << x << ',' << y << ',' << width << ',' << height << ',' << angle;
}

int Object::getX() const {
	return x;
}

int Object::getY() const {
	return y;
}

float Object::getAngle() const {
	return angle;
}

size_t Object::getHeight() const {
	return height;
}

size_t Object::getWidth() const {
	return width;
}


void Object::setX(int value) {
		x = value;
}

void Object::setY(int value){
	y = value;
}

void Object::setAngle(float value){
	angle = value;
}

void Object::setWidth(size_t value){
	width = value;
}

void Object::setHeight(size_t value){
	height = value;
}

const id_type& Object::getid() const {
	return id;
}
