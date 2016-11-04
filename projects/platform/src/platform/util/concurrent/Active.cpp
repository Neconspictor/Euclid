/******************************************************************************************
*
* Taken and adapted from http://www.grandmaster.nu/blog/?page_id=158
*
*******************************************************************************************/

#include <platform/util/concurrent/Active.hpp>
#include <boost/thread/sync_queue.hpp>

using namespace std;

namespace platform
{
	namespace util
	{

		class MessageQueue : public Active::MessageQueueWrapper
		{
		public:
			boost::sync_queue<Callback> queue;
		};


		Active::Active() :
			mIsDone(false)
		{
			mMessageQueue = new MessageQueue();
		}

		Active::~Active() {
			MessageQueue* queue = (MessageQueue*)mMessageQueue;
			if (queue->queue.closed()) {
				delete queue;
				mMessageQueue = nullptr;
				return;
			}
			send([this] { mIsDone = true; });
			mThread.join();
			delete queue;
			mMessageQueue = nullptr;
		}

		unique_ptr<Active> Active::create() {
			unique_ptr<Active> result(new Active); // this constructor is private, so make_unique would require some hassle to get to work

			result->mThread = thread(&Active::run, result.get());

			return result;
		}

		void Active::send(Callback message) {
			MessageQueue* queue = (MessageQueue*)mMessageQueue;
			queue->queue.push(move(message));
		}

		void Active::terminate()
		{
			MessageQueue* queue = (MessageQueue*)mMessageQueue;
			if (queue->queue.closed()) return;
			while (!queue->queue.empty())
				queue->queue.pull();
			send([this] { mIsDone = true; });
			mThread.join();
			queue->queue.close();
		}

		void Active::run() {
			while (!mIsDone) {
				Callback fn;
				MessageQueue* queue = (MessageQueue*)mMessageQueue;
				queue->queue.wait_pull(fn);

				fn();
			}
		}
	}
}