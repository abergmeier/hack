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
public:
	virtual ~Player();
	virtual bool operator <(const Player& player) const;
	virtual const std::string& GetUUID() const = 0;
	const std::string& GetName() const;
	virtual std::ostream& SerializeContent(const std::string& className, std::ostream& stream) const;
protected:
	Player(std::string name);
private:
	std::string _name;
	std::ostream& SerializeName( std::ostream& stream ) const;
	std::ostream& SerializeUUID( std::ostream& stream ) const;
};

} }

#endif /* PLAYER_HPP_ */
