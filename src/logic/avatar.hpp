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
	Avatar(std::string siteID);
	Avatar(std::istream& stream);
	static const std::string NAME;
	const std::string& ClassName() const override;
	std::ostream& SerializeContent(std::ostream& stream) const override;
	size_t getHitpoints() const;
	size_t getDamage() const;
	float getRadius() const;
	
private:
	// Extracts all non const fields
	void SetNonConst(std::istream& stream);
	size_t hitpoints;
	size_t damage;
	float radius;
	void setHitpoints(size_t value);
	void setDamage(size_t value);
	void hit(size_t value);


};

} }

#endif /* AVATAR_HPP_ */
