/*
* Modified code from: https://github.com/IanBullard/event_taskmanager
*
* Copyright (c) 2014 GrandMaster (gijsber@gmail)
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once

#include <nex/util/concurrent/ConcurrentQueue.hpp>
#include <nex/event/Task.hpp>
#include <nex/event/EventHandlerQueue.hpp>
#include <memory>
#include <thread>
#include <mutex>
#include <nex/event/GlobalEventChannel.hpp>
#include <condition_variable>


/**
 * A task manager is responsible for managing the live time of a task.
 * With a task manager the user can also decide to render a task on a frame
 * base, singlethreaded or multithreaded. Furthermore the task manager
 * restarts a task if it is repeating.
 *
 *  Note: when using this there must be at least one background task!
 */
class TaskManager {
public:
	typedef std::shared_ptr<Task> TaskPtr;
	typedef ConcurrentQueue<TaskPtr> TaskList;

	/**
	 * An event to stop this task manager.
	 */
	struct StopEvent {};

	/**
	 * Creates a new task manager. The parameter numThreads specifies how much
	 * threads the task manager should use for multithreaded tasks. If numThreads == 0,
	 * the task manager uses as many threads as the CPU natively supports.
	 */
	TaskManager(unsigned int numThreads = 0); //0 for autodetect

	~TaskManager();

	/**
	 * Adds a given task.
	 */
	void add(TaskPtr task);

	/**
	 * Starts this task manager. 
	 * Note: This function is blocking!
	 */
	void start();

	/**
	 * Stops this task manager. All threads controlled by this task manager will be destroyed.
	 */
	void stop();

	/**
	 * The task manager is an event handler for a StopEvent. It will stop
	 * the execution of this task manager.
	 */
	void handle(const StopEvent&);

	/**
	 * This function is called when a task provided to this task manager has finished its work.
	 */
	void handle(const Task::TaskCompleted& tc);

private:

	/**
	 * This function processes all tasks.
	 */
	void worker();

	/**
	 * Executes a given task and promotes this to the underlying event channel.
	 */
	void execute(TaskPtr task);

	/**
	 * Synchronizes multithreaded tasks that are signed to be synchronized.
	 * Basically this function waits for all running multithreaded tasks (that have the snychronization flag)
	 * to be finished. This function is intended to be called on each frame.
	 */
	void synchronize();

	std::list<std::unique_ptr<std::thread>> mThreads;
	unsigned int mNumThreads;

	bool mRunning;

	//single threaded tasks are worked off on a frame base.
	//In order to avoid a 'blocking frame' caused by adding more
	//tasks than working off, two lists are used: a write list for adding new tasks
	// and are read list for processing tasks for the current frame.
	// at the end of the frame read and write list are swaped so that the new tasks
	// will be worked off on the next frame.
	TaskList mTaskList[2];
	TaskList mBackgroundTasks;
	TaskList mSyncTasks;

	GlobalEventChannel eventChannel;

	unsigned int mReadList;
	unsigned int mWriteList;

	typedef std::mutex Mutex;
	typedef std::condition_variable Condition;
	typedef std::lock_guard<Mutex> ScopedLock;

	mutable Mutex mSyncMutex;
	mutable Mutex mSwapMutex;
	Condition mCondition;
	size_t mNumTasksToWaitFor;
};