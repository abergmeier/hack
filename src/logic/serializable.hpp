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
#include <istream>

namespace hack {
namespace state {

class Serializable {
public:
	struct String {
		static std::ostream& Serialize( std::ostream& stream, const std::string& str );
		static std::string Deserialize( std::istream& input );
	};

	virtual ~Serializable(){};
	void Serialize(std::ostream& stream) const;
	// We use ClassName instead of GetClassName because
	// §$%& VS2012 hardcodes GetClassName
	virtual const std::string& ClassName() const = 0;
protected:
	Serializable(){};
	// This has to be implemented in order to easily serialize an object
	// Most times should just call the SerializeContent overload and pass the non abstract class name
	virtual std::ostream& SerializeContent(std::ostream& stream) const = 0;
};


} }


#endif /* SERIALIZABLE_HPP_ */
