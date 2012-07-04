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
	virtual ~Object();
	virtual Object& operator = (std::istream& stream);

	// This has to be implemented in order to easily serialize an object
	// Most times should just call the Serialize overload and pass the class name
	const id_type& getid() const;
	int getX() const;
	int getY() const;
	float getAngle() const;
	size_t getHeight() const;
	size_t getWidth() const;

	void setX(int value);
	void setY(int value);
	void setAngle(float value);


private:
	static size_t NEXT_ID;
	id_type id;
	int x;
	int y;
	float angle;
	size_t width;
	size_t height;

	void Set(std::istream& stream);
	void setWidth(size_t value);
	void setHeight(size_t value);

protected:
	Object();
	// Use this constructor to deserialize object
	Object(std::istream& stream);
	virtual std::ostream& SerializeContent(std::ostream& stream) const override;
};


} }


#endif // _OBJECT_HPP_
