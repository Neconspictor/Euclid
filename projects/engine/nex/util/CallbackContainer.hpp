#pragma once

#include <functional>
#include <vector>
#include <algorithm>
#include <memory>

/**
 * A wrapper class for storing callbacks of the type CallbackType.
 */
template<class CallbackType>
class CallbackItem
{
private:
	std::function<CallbackType> callback;
public:
	using Callback = std::function<CallbackType>;

	explicit CallbackItem(const Callback& callback) : callback(callback){}

	const Callback& getCallbackFunc()
	{
		return callback;
	}

};

/**
 * A callback container stores callbacks of the type CallbackType and provides methods
 * for managing these callbacks.
 */
template <class CallbackType>
class CallbackContainer
{
public:
	using Callback = std::shared_ptr<CallbackItem<CallbackType>>;
	using ContainerItem = CallbackItem<CallbackType>;
	using CallbackFunc = std::function<CallbackType>;
private:
	std::vector<Callback> callbacks;
public:

	/**
	 * Default constructor.
	 */
	CallbackContainer()
	{
	}

	/**
	 * Adds a callback to this container.
	 */
	Callback addCallback(const CallbackFunc& callback)
	{
		Callback sharedItem =
			std::make_shared<ContainerItem>(ContainerItem(callback));
		callbacks.push_back(sharedItem);
		return sharedItem;
	}

	/**
	* Removes a callback from this container.
	*/
	void removeCallback(const Callback& item)
	{
		auto it = std::remove_if(callbacks.begin(), callbacks.end(), [&] (const Callback& current) ->bool
		{
			if (current.get() == item.get()) return true;
			return false;
		});
		callbacks.erase(it);
	}

	/**
	* Provides immutable access to the stored callbacks.
	*/
	const std::vector<Callback>& getCallbacks()
	{
		return callbacks;
	}
};