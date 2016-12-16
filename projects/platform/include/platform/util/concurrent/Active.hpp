#pragma once
/******************************************************************************************
*
* Modified code version from http://www.grandmaster.nu/blog/?page_id=158
*
*******************************************************************************************/


#include <memory>
#include <functional>
#include <thread>
#include <boost/thread/sync_queue.hpp>


namespace platform
{

	namespace util
	{

		/**
		Based on Sutters this is an Active object
		Allows threadsafe execution of void() functions in an object (by internally queuing calls)
		The object maintains its own (OS-level!) thread, so don't overuse this
		*/
		typedef std::function<void()> Callback;

		class Active {
		private:
			Active(); //only allow creation via the factory method

		public:

			/**
			 * An active object can only be created with the factory method. So the copy
			 * constructor has to be deleted.
			 */
			Active(const Active&) = delete;
			
			/**
			* An active object can only be created with the factory method. So the assignment
			* operator has to be deleted.
			*/
			Active& operator = (const Active&) = delete;

			~Active();

			/**
			 * Factory method: Creates and configures a new active object.
			 */
			static std::unique_ptr<Active> create();

			/**
			* Executes a function on this active object.
			*
			* In practice, call this with [=] lambda's -- by copying the data
			* we ensure that the data to be worked on is still live by the time
			* the active object gets to actually process it.
			*
			* [NOTE] if the callback calls non-const methods the lambda will have to be mutable...
			*/
			void send(Callback message);

			/**
			 * Clears the message queue (so some queued callbacks might not be called anymore!)
			 * and than terminates this active objects. After it has terminated it doesn't accept 
			 * no more new callbacks.
			 */
			void terminate();

		private:

			/**
			 * Starts this active thread. This function is private at it is invoked by the factory method, 
			 * since this function is intended to be called only once in its life time.
			 *
			 */
			void run(); // thread method

			bool isDone;
			boost::sync_queue<Callback> messageQueue;
			std::thread workingThread;
		};
	}
}