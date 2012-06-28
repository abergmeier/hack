/*
 * debug.hpp
 *
 *  Created on: Jun 28, 2012
 *      Author: andreas
 */

#ifndef DEBUG_HPP_
#define DEBUG_HPP_

namespace hack {


struct Debug {
	virtual ~Debug(){};
	virtual const std::string& GetCategory() const = 0;
	std::ostream& LOG      ( std::ostream& stream ) const;
	std::ostream& LOG      ( const std::string& str ) const;
	std::ostream& ERR      ( std::ostream& stream ) const;
	void         LOG_ENTRY( std::ostream& stream) const;
	void         LOG_ENTRY( const std::string& str) const;
	void         ERR_ENTRY( std::ostream& stream) const;
protected:
	Debug(){};
};

}

#endif /* DEBUG_HPP_ */
