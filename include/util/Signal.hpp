/****************************************************************
*
*	From Miguel Ibero's signal tutorial:
*   https://coderwall.com/p/u4w9ra/implementing-signals-in-c-11
*
*****************************************************************/

#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <functional>
#include <algorithm>
#include <list>
#include <memory>

template<class... F>
class SignalConnection;

template<class... F>
class ScopedSignalConnection;

template<class... F>
class SignalConnectionItem
{
public:
	typedef std::function<void(F...)> Callback;
private:
	Callback _callback;
	bool _connected;

public:
	SignalConnectionItem(const Callback& cb, bool connected = true) :
		_callback(cb), _connected(connected)
	{
	}

	void operator()(F... args)
	{
		if (_connected && _callback)
		{
			_callback(args...);
		}
	}

	bool connected() const
	{
		return _connected;
	}

	void disconnect()
	{
		_connected = false;
	}
};

template<class... F>
class Signal
{
public:
	typedef std::function<void(F...)> Callback;
	typedef SignalConnection<F...> Connection;
	typedef ScopedSignalConnection<F...> ScopedConnection;

private:
	typedef SignalConnectionItem<F...> ConnectionItem;
	typedef std::list<std::shared_ptr<ConnectionItem>> ConnectionList;

	ConnectionList _list;
	unsigned _recurseCount;

	void clearDisconnected()
	{
		_list.erase(std::remove_if(_list.begin(), _list.end(), [](std::shared_ptr<ConnectionItem>& item) {
			return !item->connected();
		}), _list.end());
	}

public:

	Signal() :
		_recurseCount(0)
	{
	}

	~Signal()
	{
		for (auto& item : _list)
		{
			item->disconnect();
		}
	}

	void operator()(F... args)
	{
		std::list<std::shared_ptr<ConnectionItem>> list;
		for (auto& item : _list)
		{
			if (item->connected())
			{
				list.push_back(item);
			}
		}
		_recurseCount++;
		for (auto& item : list)
		{
			(*item)(args...);
		}
		_recurseCount--;
		if (_recurseCount == 0)
		{
			clearDisconnected();
		}
	};

	Connection connect(const Callback& callback)
	{
		auto item = std::make_shared<ConnectionItem>(callback, true);
		_list.push_back(item);
		return Connection(*this, item);
	}

	bool disconnect(const Connection& connection)
	{
		bool found = false;
		for (auto& item : _list)
		{
			if (connection.hasItem(*item) && item->connected())
			{
				found = true;
				item->disconnect();
			}
		}
		if (found)
		{
			clearDisconnected();
		}
		return found;
	}

	void disconnectAll()
	{
		for (auto& item : _list)
		{
			item->disconnect();
		}
		clearDisconnected();
	}

	friend class Connecion;
};

template<class... F>
class SignalConnection
{
private:
	typedef SignalConnectionItem<F...> Item;

	Signal<F...>* _signal;
	std::shared_ptr<Item> _item;

public:
	SignalConnection()
		: _signal(nullptr)
	{
	}

	SignalConnection(Signal<F...>& signal, const std::shared_ptr<Item>& item)
		: _signal(&signal), _item(item)
	{
	}

	void operator=(const SignalConnection& other)
	{
		_signal = other._signal;
		_item = other._item;
	}

	virtual ~SignalConnection()
	{
	}

	bool hasItem(const Item& item) const
	{
		return _item.get() == &item;
	}

	bool connected() const
	{
		return _item->connected;
	}

	bool disconnect()
	{
		if (_signal && _item && _item->connected())
		{
			return _signal->disconnect(*this);
		}
		return false;
	}
};

template<class... F>
class ScopedSignalConnection : public SignalConnection<F...>
{
public:

	ScopedSignalConnection()
	{
	}

	ScopedSignalConnection(Signal<F...>* signal, void* callback)
		: SignalConnection<F...>(signal, callback)
	{
	}

	ScopedSignalConnection(const SignalConnection<F...>& other)
		: SignalConnection<F...>(other)
	{
	}

	~ScopedSignalConnection()
	{
		SignalConnection<F...>::disconnect();
	}

	ScopedSignalConnection & operator=(const SignalConnection<F...>& connection)
	{
		SignalConnection<F...>::disconnect();
		SignalConnection<F...>::operator=(connection);
		return *this;
	}
};

#endif