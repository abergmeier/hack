/*
 * subsystem.hpp
 *
 *  Created on: Jun 22, 2012
 *      Author: andreas
 */

#ifndef SUBSYSTEM_HPP_
#define SUBSYSTEM_HPP_

#include <future>
#include <mutex>

namespace hack {

// Class to be implemented by any Subsystem that has a clear and limited
// architecture for communication.
class Subsystem {
public:
	virtual ~Subsystem(){};
	// Start the worker loop. Should always be execute in a seperate process.
	virtual void ExecuteWorker() = 0;
protected:
	Subsystem(){};

	virtual void StopWorker() = 0;

	// Signals stop to worker
	// and wait when it stopped
	void SaveStopWorker() {
		StopWorker();
		worker.wait();
	}

	static const auto ASYNC_POLICY = std::launch::async;

	std::future<void> worker;
};

}



#endif /* SUBSYSTEM_HPP_ */
