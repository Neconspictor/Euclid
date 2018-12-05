#pragma once

#include <nex/event/EventHandlerQueue.hpp>
#include <nex/event/EventChannel.hpp>


namespace nex
{
	/**
	 * A static version of an event channel. For getting more information
	 * about event channels see platform/event/EventChannel.hpp
	 * The difference between this class and a normal event channel is, that this
	 * class acts as a singleton. All objects, that uses instances of this class
	 * are operating on the same event channel.
	 *
	 * @copydoc EventChannel
	 */
	class GlobalEventChannel : EventChannel {
	public:

		/*! @copydoc EventChannel::add(const tHandler& handler)
		 */
		template <typename tEvent, class tHandler>
		void add(tHandler& handler) {
			getHandlerQueue<tEvent>().add(handler);
		}

		/*! @copydoc EventChannel::remove(const tHandler& handler)
		*/
		template <typename tEvent, class tHandler>
		void remove(tHandler& handler) {
			getHandlerQueue<tEvent>().remove(handler);
		}

		/*! @copydoc EventChannel::broadcast(const tEvent& object)
		*/
		template <typename tEvent>
		void broadcast(tEvent object) {
			getHandlerQueue<tEvent>().broadcast(object);
		}

	private:

		/**
		 * Provides the queue for an event of type tEvent.
		 */
		template <typename tEvent>
		static EventHandlerQueue<tEvent>& getHandlerQueue()
		{
			static EventHandlerQueue<tEvent> queue;
			return queue;
		}
	};
}