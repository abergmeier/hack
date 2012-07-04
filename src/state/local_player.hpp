/*
 * local_player.h
 *
 *  Created on: Jun 22, 2012
 *      Author: andreas
 */

#ifndef _LOCAL_PLAYER_H_
#define _LOCAL_PLAYER_H_

#include <string>
#include <ostream>
#include "../logic/player.hpp"

namespace hack {
namespace state {

class LocalPlayer : public hack::logic::Player {
	std::string _uuid;
	static const std::string NAME;
public:
	LocalPlayer();
	// Commit has to be explicitly called for
	// all the changes to this object to
	// take affect. An obtrusive design
	// would also have been possible but
	// proved to be too complicated
	void Commit();
	const std::string& GetUUID() const override;
	bool IsProcessLocal() const override;
	const std::string& GetClassName() const override;
};

} }


#endif // _LOCAL_PLAYER_H_
