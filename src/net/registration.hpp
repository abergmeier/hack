
#ifndef _REGISTRATION_HPP_
#define _REGISTRATION_HPP_

#include <string>
#include <future>
#include <functional>

namespace hack {
namespace net {


class Registration {
public:
	typedef size_t port_type;
	struct Element {
		std::string uuid;
		std::string host;
		port_type port;
	};

	// Create a new registration on the server
	Registration( std::string uuid, port_type port);
	// Remove registration from server
	~Registration();

	// Returns all other registrations on the server
	static std::vector<Element> GetAll();

private:
	const std::string _uuid;
	const std::string _uri;
};

} } //namespace hack::net

#endif //_REGISTRATION_HPP_

