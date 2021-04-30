/*
 * ThreadWorker.cpp
 *
 *  Created on: Aug 19, 2016
 *      Author: ivp
 */

#include "ThreadWorker.h"

using namespace std;

namespace tmx {
namespace utils {

ThreadWorker::ThreadWorker()
{
}

ThreadWorker::~ThreadWorker()
{
	Stop();
}

void ThreadWorker::Start()
{
	if (!_thread)
	{
		_thread = new thread(&ThreadWorker::DoWork, this);
		_active = true;
	}
}

void ThreadWorker::Stop()
{
	_active = false;

	Join();

	delete _thread;
	_thread = NULL;
}

bool ThreadWorker::IsRunning()
{
	return (_thread && _active);
}

thread::id ThreadWorker::Id()
{
	static thread::id noId;
	return (_thread ? _thread->get_id() : noId);
}

bool ThreadWorker::Joinable()
{
	return (_thread ? _thread->joinable() : false);
}

void ThreadWorker::Join()
{
	if (Joinable())
		_thread->join();
}

int ThreadWorker::Size()
{
	return 0;
}

} /* namespace utils */
} /* namespace tmx */
