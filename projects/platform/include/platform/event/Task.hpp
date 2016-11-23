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
#ifndef PLATFORM_EVENT_TASK_HPP
#define PLATFORM_EVENT_TASK_HPP

#include <memory>
#include <exception>

class TaskManager;

/**
 * A task presents an executable piece that can be invoked, canceled and restarted. 
 * A task is intended to be used with a task manager (see platform/event/TaskManager.hpp)
 */
class Task {
public:
	typedef std::shared_ptr<Task> TaskPtr;

	/**
	* An event confiding that a task has started to work.
	*/
	struct TaskBeginning {
		TaskBeginning(TaskPtr t);
		TaskPtr mTask;
	};

	/**
	* An event confiding that a task has completed its work.
	*/
	struct TaskCompleted {
		TaskCompleted(TaskPtr t);
		TaskPtr mTask;
	};

	/**
	* Options specify the behaviour of a task. A task can e.g. restart after it has finished its work
	* or be executed on parallel.
	*/
	enum Options {
		/**
		 * No explicit special behaviour. The default value
		 * for any task. 
		 */
		NONE = 0x0,

		/**
		 * The task should be rexecuted after it has finished.
		 */
		REPEATING = 0x1 << 0,

		/**
		 * The task is considered to be threadsafe and should be executed on
		 * a thread pool.
		 */
		THREADSAFE = 0x1 << 1,

		/**
		 * The task should be executed once per frame.
		 */
		FRAME_SYNC = 0x1 << 2,

		/**
		 * The task isn't threadsafe and should be executed sequentially and not on a thread pool.
		 */
		SINGLETHREADED = NONE,
		
		/**
		 * The task should be processed sequentially and should be restarted once it has finished.
		 */
		SINGLETHREADED_REPEATING = REPEATING,

		/**
		 * The task is considered to be a background task and should therefore be executed on parallel on a
		 * thread pool.
		 */
		BACKGROUND = THREADSAFE,

		/**
		 * The task should be executed on parallel and restarted once it has finished.
		 */
		BACKGROUND_REPEATING = THREADSAFE | REPEATING,

		/**
		 * The task should be executed on parallel but should minimal be executed once per frame.
		 */
		BACKGROUND_SYNC = THREADSAFE | FRAME_SYNC,

		/**
		 * The task should be executed on parallel, but minimal once per frame and restarted once it has finished.
		 */
		BACKGROUND_SYNC_REPEATING = THREADSAFE | REPEATING | FRAME_SYNC,

		/**
		 * The task should use all available options.
		 */
		ALL = ~0x0
	};

	/**
	* Creates a new task. Flags can be provided to specifying the behaviour
	* of the task. The default behaviour is a task, that is singlethreaded and
	* will be repeated.
	*/
	Task(unsigned int flags = SINGLETHREADED_REPEATING);
	
	virtual ~Task();

	/**
	* The actual task, that should be executed. 
	*/
	virtual void run() = 0;

	/**
	* The tasks won't restart after it has finished its work.
	*/
	void stopRepeating() { mTaskFlags &= ~REPEATING; }

protected:
	
	// allow the task manager exclusiv access
	friend class TaskManager;

	/**
	* Provides the flags this task.
	*/
	unsigned int getTaskFlags() const;

private:
	
	unsigned int mTaskFlags;
};

#endif PLATFORM_TASK_HPP