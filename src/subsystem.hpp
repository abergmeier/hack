/*
 * subsystem.hpp
 *
 *  Created on: Jun 22, 2012
 *      Author: andreas
 */

#ifndef SUBSYSTEM_HPP_
#define SUBSYSTEM_HPP_

#include <mutex>

namespace hack {

// Class to be implemented by any Subsystem that has a clear and limited
// architecture for communication.
class Subsystem {
public:
	virtual ~Subsystem(){ std::lock_guard<std::mutex> lock( destructorMutex ); };
	// Start the worker loop. Should always be execute in a seperate process.
	virtual void ExecuteWorker() = 0;
	virtual void StopWorker() = 0;
protected:
	Subsystem(){};

	std::mutex destructorMutex;
};

}



#endif /* SUBSYSTEM_HPP_ */
