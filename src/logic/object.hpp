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

namespace hack {
namespace logic {

class Object : public entity {
public:
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
	id_type id;
	std::array<size_t, 2> position;
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
	Object(std::istream& stream);
	virtual ~Object();
	virtual Object& operator = (std::istream& stream);
	virtual void Serialize(const std::string& className, std::ostream& stream) const;
};

} }


#endif // _OBJECT_HPP_
