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
			mIsDone(false)
		{
		}

		Active::~Active() {
			if (mMessageQueue.closed()) return;
			send([this] { mIsDone = true; });
			mThread.join();
		}

		unique_ptr<Active> Active::create() {
			unique_ptr<Active> result(new Active); // this constructor is private, so make_unique would require some hassle to get to work

			result->mThread = thread(&Active::run, result.get());

			return result;
		}

		void Active::send(Callback message) {
			mMessageQueue.push(move(message));
		}

		void Active::terminate()
		{
			if (mMessageQueue.closed()) return;
			while (!mMessageQueue.empty())
				mMessageQueue.pull();
			send([this] { mIsDone = true; });
			mThread.join();
			mMessageQueue.close();
		}

		void Active::run() {
			while (!mIsDone) {
				Callback fn;

				mMessageQueue.wait_pull(fn);

				fn();
			}
		}
	}
}