/*
 * LockFreeThread.h
 *
 *  Created on: May 5, 2017
 *      Author: gmb
 */

#ifndef SRC_LOCKFREETHREAD_H_
#define SRC_LOCKFREETHREAD_H_

#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <thread>

#include "ThreadWorker.h"

namespace tmx {
namespace utils {

/**
 * A class that uses single-producer and single-consumer lock-free queues in order for optimal throughput.  The
 * capacity of each queue can be set, but it defaults to 2K.
 */
template <typename InQueueT, typename OutQueueT = InQueueT, typename Capacity = boost::lockfree::capacity<20480> >
class LockFreeThread: public ThreadWorker {
public:
	typedef InQueueT incoming_item;
	typedef OutQueueT outgoing_item;

	LockFreeThread() {}

	/**
	 * Insert into the incoming queue
	 *
	 * @param item The item to push into the queue
	 * @return True if the item was inserted
	 */
	bool push(const incoming_item &item) {
		if (accept(item) && _inQ.push(item)) {
			return true;
		}

		return false;
	}

	/**
	 * Insert directly into the outgoing queue.
	 *
	 * @param item The item to push on the queue
	 * @ret True if the item was inserted
	 */
	bool push_out(const outgoing_item &item) {
		if (_outQ.push(item)) {
			return true;
		}

		return false;
	}

	/**
	 * Remove the next item from the outgoing queue
	 *
	 * @param item The item from the queue, or undefined if nothing exists
	 * @return True if the item is defined
	 */
	bool pop(outgoing_item &item) {
		if (_outQ.pop(item)) {
			return true;
		}

		return false;
	}

	/**
	 * @return The size of the incoming queue
	 */
	uint64_t inQueueSize() {
		return _inQ.read_available();
	}

	/**
	 * @return The size of the outgoing queue
	 */
	uint64_t outQueueSize() {
		return _outQ.read_available();
	}

	int Size() {
		return inQueueSize();
	}

protected:
	/**
	 * The function that processes the items from the incoming queue.  It may or may not
	 * write to the outgoing queue.
	 */
	virtual void doWork(incoming_item &item) = 0;

	/**
	 * A function that idlles the processor when there is nothing to process
	 */
	virtual void idle() = 0;

	/**
	 * By default, every incoming item is accepted into this thread, but that behavior can be modified by
	 * overriding this method
	 * @return True to process this item, false otherwise
	 */
	virtual bool accept(const incoming_item &item) { return true; }

	void DoWork() {
		while (IsRunning()) {
			incoming_item item;
			if(_inQ.pop(item))
				doWork(item);
			else
				idle();
		}
	}

	// The lock free queues, both in and out
	boost::lockfree::spsc_queue<incoming_item, Capacity> _inQ;
	boost::lockfree::spsc_queue<outgoing_item, Capacity> _outQ;
};

}} // namespace tmx::utils

#endif /* SRC_LOCKFREETHREAD_H_ */
