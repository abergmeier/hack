/*
 * object.cpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#include "object.hpp"
#include "../include.hpp"

using namespace hack::logic;

std::ostream& hack::logic::operator <<(std::ostream& stream, const id_type& id) {
	return stream << '{'
	               << id.global_id << ','
	               << id.local_id
	               << '}';
}

std::istream& hack::logic::operator >>(std::istream& stream, id_type& id) {
	char term;
	stream .get(term); //{
	stream >> id.global_id;
	stream .get(term); //,
	stream >> id.local_id;
	stream .get(term); //}
	return stream;
}

std::ostream& hack::logic::operator <<(std::ostream& stream, const Object::Position& position) {
	return _left_shift_operator(stream, position);
}
std::istream& hack::logic::operator >>(std::istream& stream, Object::Position& position) {
	return _right_shift_operator(stream, position);
}

id_type::id_type(std::istream& stream) {
	stream >> *this;
}

bool id_type::operator <(const id_type& other) const {
	if( global_id == other.global_id )
		return local_id < other.local_id;

	return global_id < other.global_id;
}


Object::Object(std::istream& stream) :
	id(stream)
{
	Set(stream);
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
