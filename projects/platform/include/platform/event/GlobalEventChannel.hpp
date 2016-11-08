#ifndef PLATFORM_GLOBAL_EVENT_CHANNEL_HPP
#define PLATFORM_GLOBAL_EVENT_CHANNEL_HPP

#include <platform/event/EventHandlerQueue.hpp>

class GlobalEventChannel {
public:

	template <typename tEvent, class tHandler>
	void add(tHandler& handler) {
		getHandlerQueue<tEvent>().add(handler);
	}

	template <typename tEvent, class tHandler>
	void remove(const tHandler& handler) {
		getHandlerQueue<tEvent>().remove(handler);
	}

	template <typename tEvent>
	void broadcast(const tEvent& object) {
		getHandlerQueue<tEvent>().broadcast(object);
	}

private:

	template <typename tEvent>
	static EventHandlerQueue<tEvent>& getHandlerQueue()
	{
		static EventHandlerQueue<tEvent> queue;
		return queue;
	}
};

#endif