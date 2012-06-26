#pragma once

#ifndef _ENTITY_HPP_
#define _ENTITY_HPP_

#include <string>
#include <utility>
//#include <cstdlib> // hier ???

namespace hack {
namespace logic {

struct id_type {
	size_t peerid;
	size_t localid;
};


class entity {

public:
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
entity();

};

}
}

#endif // _ENTITY_HPP_


// Const Version der Funktionen ?