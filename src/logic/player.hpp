/*
 * player.hpp
 *
 *  Created on: Jun 18, 2012
 *      Author: andreas
 */

#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <string>
#include <functional>
#include <array>
#include "serializable.hpp"

namespace hack {
namespace logic {

class Player : public hack::state::Serializable {
	std::string _name;
protected:
	Player(std::string name);
public:
	virtual ~Player();
	virtual bool operator <(const Player& player) const;
	virtual const std::string& GetUUID() const = 0;
	const std::string& GetName() const;
	virtual std::ostream& SerializeContent(const std::string& className, std::ostream& stream) const;
};

} }

#endif /* PLAYER_HPP_ */
