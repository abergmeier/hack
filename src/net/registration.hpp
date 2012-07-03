
#ifndef _REGISTRATION_HPP_
#define _REGISTRATION_HPP_

#include <string>
#include <future>
#include <functional>
#include <condition_variable>
#include "../subsystem.hpp"

namespace hack {
namespace net {


class Registration : public hack::Subsystem {
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

	void ExecuteWorker() override;
	void StopWorker() override;

private:
	const std::string _uuid;
	const std::string _uri;
	bool _isPinging;
	std::condition_variable _sleepCondition;
};

} } //namespace hack::net

#endif //_REGISTRATION_HPP_

