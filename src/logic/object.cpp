/*
 * object.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#include "object.hpp"
#include "../include.hpp"

using namespace hack::logic;

std::ostream& hack::logic::operator <<(std::ostream& stream, const Object::Position& position) {
	return _left_shift_operator(stream, position);
}
std::istream& hack::logic::operator >>(std::istream& stream, Object::Position& position) {
	return _right_shift_operator(stream, position);
}

size_t Object::NEXT_ID = 1;

Object::Object() :
	entity(),
	hack::state::Serializable(),
	id("", NEXT_ID)
{
	++NEXT_ID;
}

Object::Object(std::istream& stream) :
	id(stream)
{
	Set(stream);
}

Object::~Object() {
}

void Object::Set(std::istream& stream) {
	stream >> _position;
}

Object& Object::operator = (std::istream& stream) {
	Set(stream);
	return *this;
}

std::ostream& Object::SerializeContent(const std::string& className, std::ostream& stream) const {
	return Serializable::SerializeContent( className, stream )
	<< ',' << id << ',' << _position;
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
