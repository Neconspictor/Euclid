#pragma once

#include <functional>
#include <vector>
#include <algorithm>
#include <memory>

namespace nex
{
	/**
	* A callback collection stores callbacks of a given type and provides methods
	* for managing its callbacks.
	*/
	template <class CallbackType>
	class CallbackCollection
	{
	public:
		using Callback = std::function<CallbackType>;
		using Handle = std::shared_ptr<Callback>;
	private:
		std::vector<Handle> callbacks;
	public:

		/**
		* Adds a callback to this container.
		*/
		Handle addCallback(const Callback& callback)
		{
			Handle sharedItem = std::make_shared<Callback>(callback);
			callbacks.push_back(sharedItem);
			return sharedItem;
		}

		/**
		* Removes a callback from this container.
		*/
		void removeCallback(const Handle& handle)
		{
			auto it = std::remove_if(callbacks.begin(), callbacks.end(), [&](const Handle& current) ->bool
			{
				return static_cast<bool>(current.get() == handle.get());
			});
			if (it != callbacks.end())
				callbacks.erase(it);
		}

		/**
		* Provides immutable access to the stored callbacks.
		*/
		const std::vector<Handle>& getCallbacks()
		{
			return callbacks;
		}
	};
}