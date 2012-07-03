#ifndef AVATAR_HPP_
#define AVATAR_HPP_

#include <istream>
#include <ostream>
#include <string>
#include "object.hpp"

namespace hack {
namespace logic {

class Avatar : public Object {
public:
	Object& operator = (std::istream& stream) override;
	Avatar(); 
	Avatar(std::istream& stream);
	static const std::string NAME;
	std::ostream& SerializeContent(std::ostream& stream) const override;
	std::ostream& SerializeContent(const std::string& className, std::ostream& stream) const override;
	size_t getHitpoints();
	size_t getDamage();
	
private:
	void Set(std::istream& stream);
	size_t hitpoints;
	size_t damage;
	void setHitpoints(size_t value);
	void setDamage(size_t value);
	void hit(size_t value);
	

};

} }

#endif /* AVATAR_HPP_ */
