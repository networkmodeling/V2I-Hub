/*
 * ThreadGroup.h
 *
 *  Created on: May 5, 2017
 *      Author: gmb
 */

#ifndef SRC_THREADGROUP_H_
#define SRC_THREADGROUP_H_

#include <atomic>
#include <boost/algorithm/string.hpp>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <thread>
#include "ThreadWorker.h"
#include <vector>

namespace tmx {
namespace utils {

/*
 * A class that manages a set of running threads so that they can be created, started and
 * stopped all in unison. Threads can only be added to the group,thus increasing the size,
 * and the individual threads can be modified, but the thread group cannot currently be shrunk.
 */
class ThreadGroup {
public:
	ThreadGroup() { }
	ThreadGroup(size_t size): _threads(size) { }
	ThreadGroup(const ThreadGroup &copy): _threads(copy._threads) { }

	/**
	 * @return The number of threads in the group.
	 */
	inline size_t size() const {
		return _threads.size();
	}

	/**
	 * Locate the specified thread within the thread group, and return the id
	 * of the thread in the group.
	 *
	 * @param threadId The thread to locate
	 * @return The group thread id
	 */
	inline int this_thread(std::thread::id threadId) {
		size_t total = _threads.size();
		for (size_t i = 0; i < total; i++) {
			if (_threads[i] && _threads[i]->Id() == threadId)
				return i;
		}

		return -1;
	}

	/**
	 * Locate the current running thread withing the thread group, and return the
	 * id of the thread in the group
	 *
	 * @return The group thread id
	 */
	inline int this_thread() {
		return this_thread(std::this_thread::get_id());
	}

	/**
	 * Add a new thread to the end of the queue, and return the id of that
	 * thread in the group.
	 *
	 * @return The group thread id, or -1 if the thread could not be added
	 */
	inline int push_back(ThreadWorker *thread) {
		if (!thread)
			return -1;

		_threads.push_back(thread);
		return this_thread(thread->Id());
	}

	/**
	 * @param Thread id in the group
	 * @returns A reference to the thread object at that id
	 */
	inline ThreadWorker *operator[](size_t n) {
		return _threads[n];
	}

	/**
	 * @param Thread id in the group
	 * @returns A const reference to the thread object at that id
	 */
	inline ThreadWorker *operator[](size_t n) const {
		return _threads[n];
	}

	void Start() {
		for (size_t i = 0; i < _threads.size(); i++)
			if (_threads[i])
				_threads[i]->Start();
	}

	void Stop() {
		for (size_t i = 0; i < _threads.size(); i++) {
			if (_threads[i])
				_threads[i]->Stop();
		}
	}

	bool IsRunning() {
		for (size_t i = 0; i < _threads.size(); i++)
			if (!_threads[i] || !_threads[i]->IsRunning())
				return false;

		return true;
	}

	void Clear() {
		Stop();

		for (size_t i = 0; i < _threads.size(); i++)
			delete _threads[i];

		_threads.clear();
	}

private:
	std::deque<ThreadWorker *> _threads;
};

/**
 * A template class that defines how tasks are assigned to individual threads.
 * Assignment is done based on an assigned group
 */
template <typename GroupT, typename IdentifierT = GroupT>
class ThreadGroupAssignment {
public:
	typedef GroupT group_type;
	typedef IdentifierT id_type;
	static constexpr size_t max_groups = ::pow(2, 8 * sizeof(group_type));
	static constexpr size_t max_ids = ::pow(2, 8 * sizeof(id_type));

	ThreadGroupAssignment(ThreadGroup &threads):
			_threads(threads),
			_strategy((ThreadGroupAssignmentStrategy)0) {
		srand(time (NULL));

		// Initialize the queue assignments
		for (size_t i = 0; i < max_groups; i++) {
			for (size_t j = 0; j < max_ids; j++) {
				assignments[i][j].count = 0;
				assignments[i][j].threadId = -1;
			}
		}
	}

	ThreadGroup &Group() { return _threads; }

	/**
	 * Assign a group and id to the next available thread.  If the group and id are set to
	 * any non-zero value and there already is an thread assignment for that group and id, then
	 * the existing thread assignment will be used.  If no thread assignment exists, or group and
	 * id are set to zero, indicating thread assignment should be ignored, then the item is assigned
	 * to a thread based on the specified assignment strategy, either round-robin (default), random,
	 * or shortest-queue.
	 *
	 * @param group The group identifier, or 0 for no group
	 * @param id The unique identifier in the group, or 0 for no identifier
	 * @return The thread id of the resulting assignment, or -1 if no assignment can be made
	 * @see set_strategy(std::string)
	 */
	int assign(group_type group, id_type id) {
		static std::atomic<uint32_t> next {0};

		if (_threads.size() == 0)
			return -1;

		int tId = -1;

		// Need this here to ensure no pre-mature free up of the thread ID
		assignments[group][id].count++;

		tId = assignments[group][id].threadId;

		// If no group and no id, then any existing thread assignment should be ignored
		if (tId < 0 || (group == 0 && id == 0)) {
			// No thread assignment.  Assign using assignment strategy
			switch (_strategy) {
			case strategy_RoundRobin:
				tId = next;
				if (++next >= _threads.size())
					next = 0;
				break;
			case strategy_Random:
				tId = rand() % _threads.size();
				break;
			case strategy_ShortestQueue:
				tId = 0;
				for (size_t i = 1; i < _threads.size(); i++) {
					if (_threads[i] && _threads[tId] &&
							_threads[i]->Size() < _threads[tId]->Size())
						tId = i;
				}
				break;
			default:
				tId = 0;
			}

			assignments[group][id].threadId = tId;
		}

		return tId;
	}

	/**
	 * Remove a thread assignment for a specific group and id, assuming no more tasks
	 * exists for that thread to finish.  If this is never called, then the thread
	 * assignment for the group and id will last forever.
	 *
	 * @param group The group identifier
	 * @param id The unique identifier in the group
	 */
	void unassign(group_type group, id_type id) {
		if (!(--assignments[group][id].count))
			assignments[group][id].threadId = -1;
	}

	/**
	 * Sets an assignment strategy by name, which is one of:
	 * 		RoundRobin
	 * 		Random
	 * 		ShortestQueue
	 * The string compare is case-insensitive.
	 * @param strategy The new strategy
	 */
	void set_strategy(std::string strategy) {
		_strategy = get_strategy(strategy);
	}

private:
	ThreadGroup &_threads;

	struct source_info {
		std::atomic<uint64_t> count;
		std::atomic<int> threadId;
	};

	source_info assignments[max_groups][max_ids];

	enum ThreadGroupAssignmentStrategy {
		strategy_RoundRobin,
		strategy_Random,
		strategy_ShortestQueue,
		strategy_END
	};

	ThreadGroupAssignmentStrategy _strategy;

	ThreadGroupAssignmentStrategy get_strategy(std::string &str) {
		static std::vector<std::string> allStrategies;

		if (allStrategies.empty()) {
			allStrategies.resize(strategy_END);

			// Initialize all the strategies
#define LOAD(X) allStrategies[strategy_ ## X] = #X
			LOAD(RoundRobin);
			LOAD(Random);
			LOAD(ShortestQueue);
#undef LOAD
		}

		for (size_t i = 0; i < allStrategies.size(); i++) {
			if (boost::iequals(allStrategies[i], str))
				return (ThreadGroupAssignmentStrategy)i;
		}

		// Return existing strategy
		return _strategy;
	}
};


}} // namespace tmx::utils

#endif /* SRC_THREADGROUP_H_ */
