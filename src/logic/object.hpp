/*
 * object.hpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

namespace hack {
namespace logic {

class Object {
public:
	// This has to be implemented in order to easily serialize an object
	// Most times should just call the Serialize overload and pass the class name
	virtual void Serialize(std::ostream& stream) const = 0;
private:
	size_t id;
	std::array<size_t, 2> position;
	void Set(std::istream& stream);
protected:
	Object(std::istream& stream);
	virtual ~Object();
	virtual Object& operator = (std::istream& stream);
	virtual void Serialize(const std::string& className, std::ostream& stream) const;
};

} }


#endif // _OBJECT_HPP_
