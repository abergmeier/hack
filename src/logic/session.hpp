/*
 * session.hpp
 *
 *  Created on: Jun 12, 2012
 *      Author: andreas
 */

#ifndef SESSION_HPP_
#define SESSION_HPP_

namespace hack {

class Session {

};

class Sessions {
	static std::vector<Session> _sessions;
public:
	Sessions() = delete;
	static const std::vector& Get();
};

} // namespace hack

#endif /* SESSION_HPP_ */
