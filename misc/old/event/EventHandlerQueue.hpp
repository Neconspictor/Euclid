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
#include <nex/event/EventBridge.hpp>
#include <list>
#include <memory>


namespace nex
{
	/**
	 * The responsibilities of this class are the storing of events of type tEvent and managing handler
	 * for this event type. This class is not intended to be used by publicity. The public interface for
	 * events are platform/event/EventChannel and platform/event/GlobalEventChannel.
	 */
	template <typename tEvent>
	class EventHandlerQueue {
	private: //the entire class is private, so no unauthored class may use it

		// event channels are allowed to access this class
		friend class EventChannel;
		friend class GlobalEventChannel;

		typedef EventHandler<tEvent> EventHandlerType;

		typedef std::shared_ptr<EventHandlerType> EventHandlerPtr;

		typedef std::list<EventHandlerPtr> HandlerList;
		typedef typename HandlerList::iterator HandlerIterator;
		typedef typename HandlerList::const_iterator ConstHandlerIterator;

		HandlerList mHandlerList;

		/**
		 * Adds an event handler, that is willing to listen to events of type tEvent.
		 */
		template <class tHandler>
		void add(tHandler& handler) {
			mHandlerList.push_back(std::make_shared<EventBridge<tEvent, tHandler>>(handler));
		}

		/**
		 * Broadcasts an event to all registered event handlers.
		 */
		void broadcast(tEvent object) {
			ConstHandlerIterator next;
			for (ConstHandlerIterator it = mHandlerList.begin(); it != mHandlerList.end(); ) {

				//the handle function might invalidate the iterator, so make a copy and advance immediately
				next = it++;
				(*next)->handle(object);
			}
		}

		/**
		 * Comparison predicate class. It decides if two event handlers are equal or not.
		 * This class is needed for removing a handler.
		 */
		template <class tHandler>
		class IsEqualTo {
		public:
			typedef EventBridge<tEvent, tHandler> BridgeType;

			const tHandler& mHandler;

			IsEqualTo(const tHandler& h) : mHandler(h) {}

			bool operator()(EventHandlerPtr ptr) {
				return ((*std::static_pointer_cast<BridgeType>(ptr)) == mHandler);
			}
		};

		/**
		 * Removes an event handler if it was registered to this queue before.
		 */
		template <class tHandler>
		void remove(const tHandler& handler) {
			IsEqualTo<tHandler> pts(handler);
			mHandlerList.remove_if(pts);
		}
	};
}