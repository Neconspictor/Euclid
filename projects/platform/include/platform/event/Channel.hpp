#ifndef CHANNEL_HPP
#define CHANNEL_HPP

/************************************************************************************
 *
 *	This class is from GrandMaster's tutorial:
 *  http://www.grandmaster.nu/blog/?page_id=191
 * 
 ************************************************************************************/

#include <vector>
#include <functional>
#include <mutex>
namespace platform {
	class Channel {
	public:
		template <typename T>
		class InternalChannel;

		template <typename tMessage, typename tHandler>
		static void add(tHandler* handler) {
			// typically, the handler type is derived while the message type would be explicit
			// e.g. Channel<MyEvent>::add(this);
			InternalChannel<tMessage>::instance().add(handler); //forward to the appropriate queue
		}

		template <typename tMessage, typename tHandler>
		static void remove(tHandler* handler) {
			InternalChannel<tMessage>::instance().remove(handler);
		}

		template <typename tMessage>
		static void broadcast(const tMessage& message) {
			// usually no need to be explicit, the message type can be derived at compiletime
			InternalChannel<tMessage>::instance().broadcast(message);
		}

	private:
		template <typename tMessage>
		class InternalChannel {
		public:
			typedef std::function<void(const tMessage&)> Handler;

			static InternalChannel& instance() {
				static InternalChannel result;
				return result;
			}

			template <typename tHandler>
			void add(tHandler* handler) {
				std::lock_guard<std::mutex> lock(mMutex);

				mHandlers.push_back([handler](const tMessage& msg) { (*handler)(msg); });
				mOriginalPtrs.push_back(handler);
			}

			template <typename tHandler>
			void remove(tHandler* handler) {
				std::lock_guard<std::mutex> lock(mMutex);

				auto it = std::find(mOriginalPtrs.begin(), mOriginalPtrs.end(), handler);
				if (it == mOriginalPtrs.end())
				{
					std::cout << "Tried to remove a handler that was not in the handler list" << std::endl;
					//throw std::runtime_error("Tried to remove a handler that was not in the handler list");
					return;
				}

				auto idx = it - mOriginalPtrs.begin();

				mHandlers.erase(mHandlers.begin() + idx);
				mOriginalPtrs.erase(it);
			}

			void broadcast(const tMessage& msg) {
				std::vector<Handler> localQueue(mHandlers.size());

				{
					std::lock_guard<std::mutex> lock(mMutex);
					localQueue = mHandlers;
				}

				for (auto& handler : localQueue)
					handler(msg);
			}

		private:
			std::mutex mMutex;
			std::vector<Handler> mHandlers;
			std::vector<void*> mOriginalPtrs;
		};
	};
}
#endif