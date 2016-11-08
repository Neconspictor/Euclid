#ifndef ACTIVE_HPP
#define ACTIVE_HPP
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
			Active(const Active&) = delete;
			Active& operator = (const Active&) = delete;

			~Active();

			static std::unique_ptr<Active> create(); // factory method (make sure 

			/**
			In practice, call this with [=] lambda's -- by copying the data
			we ensure that the data to be worked on is still live by the time
			the active object gets to actually process it.

			[NOTE] if the callback calls non-const methods the lambda will have to be mutable... -_-
			*/
			void send(Callback message);

			void terminate();

		private:
			void run(); // thread method

			bool mIsDone;
			boost::sync_queue<Callback> mMessageQueue;
			std::thread mThread;
		};
	}
}


#endif