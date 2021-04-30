/*
 * ThreadWorker.h
 *
 *  Created on: Aug 19, 2016
 *      Author: ivp
 */

#ifndef SRC_THREADWORKER_H_
#define SRC_THREADWORKER_H_

#include <atomic>
#include <thread>

namespace tmx {
namespace utils {

/**
 * Abstract class that manages a thread for performing work.
 */
class ThreadWorker {
public:
	/***
	 * Construct a new background worker.  The background worker is stopped,
	 * and will invoke the DoWork() function when started.
	 */
	ThreadWorker();
	virtual ~ThreadWorker();

	/**
	 * Start the thread worker.
	 */
	virtual void Start();

	/**
	 * Stop the thread worker.
	 */
	virtual void Stop();

	/**
	 * @return True if the background worker is currently running.
	 */
	virtual bool IsRunning();

	/**
	 * @return The thread id of this worker
	 */
	virtual std::thread::id Id();

	/**
	 * @return True if the background worker can be joined.
	 */
	virtual bool Joinable();

	/**
	 * Join the background worker
	 */
	virtual void Join();

	/**
	 * @return The size of the background worker task queue, if one exists
	 */
	virtual int Size();


protected:
	/**
	 * The parent class implements this method to do the work in the spawned thread.
	 * DoWork should exit when the _stopThread variable has a value of false.
	 * The example below can be used as a template.
	 *
	 * void ParentClass::DoWork()
	 * {
	 * 	 while (!_stopThread)
	 *     this_thread::sleep_for(chrono::milliseconds(50));
	 * }
	 *
	 */
	virtual void DoWork() = 0;

	/**
	 * When this value is set to false (by the StopWorker method), the DoWork method should exit.
	 */
	std::atomic<bool> _active {false};

	std::thread *_thread = NULL;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_THREADWORKER_H_ */
