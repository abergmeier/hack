/*
 * endpoint.hpp
 *
 *  Created on: Jun 15, 2012
 *      Author: andreas
 */

#ifndef ENDPOINT_HPP_
#define ENDPOINT_HPP_

#include <vector>
#include <functional>
#include <queue>

namespace hack {
namespace net {

class Endpoint {
protected:
	Endpoint(){}; //Use as interface only
public:
	virtual ~Endpoint(){};
	typedef Network::buffer_type buffer_type;
	virtual void SendTo(buffer_type buffer, std::function<void()> callback) = 0;
	virtual std::queue<buffer_type> GetFrom() = 0;
};

} }


#endif /* ENDPOINT_HPP_ */
