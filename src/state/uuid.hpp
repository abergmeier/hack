/*
 * uuid.hpp
 *
 *  Created on: Jun 26, 2012
 *      Author: andreas
 */

#ifndef UUID_HPP_
#define UUID_HPP_

#include <string>

namespace hack {

struct UUID {
	static std::string Generate();
};

}


#endif /* UUID_HPP_ */
