/******************************************************************************************
*
* Taken and adapted from http://www.grandmaster.nu/blog/?page_id=158
*
*******************************************************************************************/

#include <platform/util/concurrent/Active.hpp>

using namespace std;

namespace platform
{
	namespace util
	{
		Active::Active() :
			isDone(false)
		{
		}

		Active::~Active() {
			if (messageQueue.closed()) return;
			send([this] { isDone = true; });
			workingThread.join();
		}

		unique_ptr<Active> Active::create() {
			unique_ptr<Active> result(new Active); // this constructor is private, so make_unique would require some hassle to get to work

			result->workingThread = thread(&Active::run, result.get());

			return result;
		}

		void Active::send(Callback message) {
			messageQueue.push(move(message));
		}

		void Active::terminate()
		{
			if (messageQueue.closed()) return;
			//while (!messageQueue.empty())
			//	messageQueue.pull();
			send([this] { isDone = true; });
			workingThread.join();
			messageQueue.close();
		}

		void Active::run() {
			while (!isDone) {
				Callback fn;

				messageQueue.wait_pull(fn);

				fn();
			}
		}
	}
}