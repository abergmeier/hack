/*
 * object.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#include "object.hpp"
#include "../include.hpp"

using namespace hack::logic;


size_t Object::NEXT_ID = 1;

Object::Object() :
	entity(),
	hack::state::Serializable(),
	id("", NEXT_ID), x(0), y(0)
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
	 stream.get();
	 stream >> x;
	 stream.get();
	 stream >> y;
	 stream.get();
	 stream >> width;
	 stream.get();
	 stream >> height;
	 stream.get();
	 stream >> angle;
}

Object& Object::operator = (std::istream& stream) {
	Set(stream);
	return *this;
}

std::ostream& Object::SerializeContent(const std::string& className, std::ostream& stream) const {
	return Serializable::SerializeContent( className, stream )
		<< ',' << id << ',' << x << ',' << y << ',' << width << ',' << height << ',' << angle ;
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
