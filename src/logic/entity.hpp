#pragma once

#ifndef _ENTITY_HPP_
#define _ENTITY_HPP_

#include <string>
#include <utility>
//#include <cstdlib> // hier ???

namespace hack {
namespace logic {



struct id_type {
	id_type(std::string global, size_t local);
	id_type(std::istream& stream );
	id_type(const id_type& other);
	id_type(id_type&& other      );
	id_type& operator=(const id_type& other);
	id_type& operator=(id_type&& other      );
	bool operator <(const id_type& other) const;
	std::string global_id;
	size_t local_id;
};

std::ostream& operator <<(std::ostream& stream, const id_type& id);
std::istream& operator >>(std::istream& stream, id_type& id);

class entity {

public:
	virtual ~entity(){};
	// We use ClassName instead of GetClassName because
	// §$%& VS2012 hardcodes GetClassName
	virtual const std::string& ClassName() const = 0;
	virtual const id_type& getid() const = 0;
	virtual int getX() const = 0;
	virtual int getY() const = 0;
	virtual float getAngle() const = 0;
	virtual size_t getHeight() const= 0;
	virtual size_t getWidth() const = 0;


private:
virtual void setX(int value) = 0;
virtual void setY(int value) = 0;
virtual void setAngle(float value) = 0;
virtual void setWidth(size_t value) = 0;
virtual void setHeight(size_t value) = 0;

protected:
	entity(){};

};

}
}

#endif // _ENTITY_HPP_

