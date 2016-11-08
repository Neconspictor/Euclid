#ifndef CALLBACK_CONTAINER_HPP
#define CALLBACK_CONTAINER_HPP

#include <functional>
#include <vector>
#include <algorithm>
#include <memory>

template<class CallbackType>
class CallbackItem
{
private:
	std::function<CallbackType> callback;
public:
	using Callback = std::function<CallbackType>;

	explicit CallbackItem(const Callback& callback) : callback(callback){}

	const Callback& getCallback()
	{
		return callback;
	}

};

template <class CallbackType>
class CallbackContainer
{
public:
	using SharedItem = std::shared_ptr<CallbackItem<CallbackType>>;
	using Item = CallbackItem<CallbackType>;
	using Callback = std::function<CallbackType>;
private:
	std::vector<SharedItem> callbacks;
public:

	CallbackContainer()
	{
	}

	SharedItem addCallback(const Callback& callback)
	{
		SharedItem sharedItem =
			std::make_shared<Item>(Item(callback));
		callbacks.push_back(sharedItem);
		return sharedItem;
	}

	void removeCallback(const SharedItem& item)
	{
		auto it = std::remove_if(callbacks.begin(), callbacks.end(), [&] (const SharedItem& current) ->bool
		{
			if (current.get() == item.get()) return true;
			return false;
		});
		callbacks.erase(it);
	}

	const std::vector<SharedItem>& getCallbacks()
	{
		std::vector<SharedItem> result;
		for (auto elem : callbacks) 
			result.push_back(elem);
		return result;
	}
};

#endif