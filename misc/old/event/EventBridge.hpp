/*
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

#ifndef EVENT_BRIDGE_HPP
#define EVENT_BRIDGE_HPP

#include <nex/event/EventHandler.hpp>

namespace nex
{

	template <class> class EventHandlerQueue;

	template <typename tEventType, class tHandler>
	class EventBridge : public EventHandler<tEventType> {
	public:

		EventBridge(tHandler& handler);

	private:
		friend class EventHandlerQueue<tEventType>;

		tHandler& mHandler;

		void handle(tEventType& type) override;

	public:
		bool operator == (tHandler& handler) const;
	};

	//Implementation
	template <typename T, class U>
	EventBridge<T, U>::EventBridge(U& handler) :
		mHandler(handler)
	{
	}

	template <typename T, class U>
	void EventBridge<T, U>::handle(T& object) {
		mHandler.handle(object);
	}

	template <typename T, class U>
	bool EventBridge<T, U>::operator == (U& handler) const {
		return ((&mHandler) == (&handler));
	}
}

#endif
