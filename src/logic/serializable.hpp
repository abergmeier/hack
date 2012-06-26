/*
 * serializable.hpp
 *
 *  Created on: Jun 25, 2012
 *      Author: andreas
 */

#ifndef SERIALIZABLE_HPP_
#define SERIALIZABLE_HPP_

#include <string>
#include <ostream>

namespace hack {
namespace state {

class Serializable {
public:
	virtual ~Serializable(){};
	void Serialize(std::ostream& stream) const;
protected:
	Serializable(){};
	// This has to be implemented in order to easily serialize an object
	// Most times should just call the SerializeContent overload and pass the non abstract class name
	virtual std::ostream& SerializeContent(std::ostream& stream) const = 0;
	virtual std::ostream& SerializeContent(const std::string& className, std::ostream& stream) const;
};


} }


#endif /* SERIALIZABLE_HPP_ */
