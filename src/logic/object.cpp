/*
 * object.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#include "object.hpp"

using namespace hack::logic;

Object::Object(std::istream& stream) {
	stream >> id;
	Set(stream);
}

void Object::Set(std::istream& stream) {
	stream >> position;
}

Object& Object::operator = (std::istream& stream) {
	Set(stream);
	return *this;
}

void Object::Serialize(const std::string& className, std::ostream& stream) const {
	stream << className;
	stream << id;
	stream << position;
}

int Object::getX() const {
	return x;
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