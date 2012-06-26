/*
 * object.hpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <istream>
#include <ostream>
#include <array>
#include <string>
#include "entity.hpp"
#include "serializable.hpp"

namespace hack {
namespace logic {

class Object : public entity, public hack::state::Serializable {
public:
	// Use a class so we can have working operator deduction
	class Position : public std::array<size_t, 2> {
	};
	virtual ~Object();
	virtual Object& operator = (std::istream& stream);

	// This has to be implemented in order to easily serialize an object
	// Most times should just call the Serialize overload and pass the class name
	virtual void Serialize(std::ostream& stream) const = 0;
	const id_type& getid() const;
	int getX() const;
	int getY() const;
	float getAngle() const;
	size_t getHeight() const;
	size_t getWidth() const;

private:
	static size_t NEXT_ID;
	id_type id;
	Position _position;
	int x;
	int y;
	float angle;
	size_t width;
	size_t height;

	void Set(std::istream& stream);
	void setX(int value);
	void setY(int value);
	void setAngle(float value);
	void setWidth(size_t value);
	void setHeight(size_t value);

protected:
	Object();
	// Use this constructor to deserialize object
	Object(std::istream& stream);
	virtual std::ostream& SerializeContent(const std::string& className, std::ostream& stream) const;
};

std::ostream& operator <<(std::ostream& stream, const Object::Position& id);
std::istream& operator >>(std::istream& stream, Object::Position& id);

} }


#endif // _OBJECT_HPP_
