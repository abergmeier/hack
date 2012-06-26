/*
 * subsystem.hpp
 *
 *  Created on: Jun 22, 2012
 *      Author: andreas
 */

#ifndef SUBSYSTEM_HPP_
#define SUBSYSTEM_HPP_

namespace hack {

// Class to be implemented by any Subsystem that has a clear and limited
// architecture for communication.
class Subsystem {
public:
	virtual ~Subsystem();
	// Start the worker loop. Should always be execute in a seperate process.
	virtual void ExecuteWorker() = 0;
protected:
	Subsystem();
};

}



#endif /* SUBSYSTEM_HPP_ */
