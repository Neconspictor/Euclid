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
#ifndef EVENT_CHANNEL_HPP
#define EVENT_CHANNEL_HPP

#include <platform/event/EventHandlerQueue.hpp>
#include <unordered_map>
#include <boost/any.hpp>
#include <typeindex>

/**
 * This class stores events in seperate queues(channels) through their event type 
 * and thus processes events independently from other event types. 
 * Event handlers can sign up to one or more event channels and will be notified 
 * to events of these channels. Splitting up events into channels gives the advantage
 * that each handler only gets notified to certain event messages (and not all).
 */
class EventChannel {
public:

	/**
	 * Adds a handler for a specific event type. the handler will be notified by 
	 * all broadcasted events having that event type. An event handler has to implement
	 * the following function, in order to get notified: void handle(const tEvent&)
	 * Whereby tEvent is the class/struct name of the event type, that should be handled.
	 */
	template <typename tEvent, class tHandler>
	void add(tHandler& handler) {
		EventHandlerQueue<tEvent>& queue = getQueue<tEvent>();
		queue.add(handler);
	}

	/**
	* Removes a given event handler from events of the type tEvent.
	* The handler don't get informed anymore about tEvent events.
	* This function does nothing, if the event handler wasn't added before.
	*/
	template <typename tEvent, class tHandler>
	void remove(const tHandler& handler) {
		EventHandlerQueue<tEvent>& queue = getQueue<tEvent>();
		queue.remove(handler);
	}

	/**
	* Broadcasts an event of type tEvent. All registered event handlers that
	* are listening for events of type tEvent will be handling this event.
	*/
	template <typename tEvent>
	void broadcast(const tEvent& object) {
		EventHandlerQueue<tEvent>& queue = getQueue<tEvent>();
		queue.broadcast(object);
	}

private:

	/**
	* Provides the event queue for a event of type tEvent.
	*/
	template <typename tEvent>
	EventHandlerQueue<tEvent>& getQueue()
	{
		auto key = std::type_index(typeid(EventHandlerQueue<tEvent>));
		auto it = queueMap.find(key);
		if (it == queueMap.end())
		{
			it = queueMap.emplace(key, boost::any(EventHandlerQueue<tEvent>())).first;
		}
		return boost::any_cast<EventHandlerQueue<tEvent>&>(it->second);
	}

	/**
	*
	*/
	std::unordered_map<std::type_index, boost::any> queueMap;
};

#endif