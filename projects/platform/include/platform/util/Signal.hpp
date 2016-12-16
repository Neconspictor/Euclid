/****************************************************************
*
*	From Miguel Ibero's signal tutorial:
*   https://coderwall.com/p/u4w9ra/implementing-signals-in-c-11
*
*****************************************************************/

#pragma once

#include <functional>
#include <algorithm>
#include <list>
#include <memory>

template<class... F>
class SignalConnection;

template<class... F>
class ScopedSignalConnection;

/**
 * A signal connection item holds information about a slot that is connected to a signal.
 */
template<class... F>
class SignalConnectionItem
{
public:
	typedef std::function<void(F...)> Slot;
private:
	Slot slot;
	bool isConnected;

public:
	SignalConnectionItem(const Slot& slot, bool connected = true) :
		slot(slot), isConnected(connected)
	{
	}

	/**
	 * Executes the slot held by this item if the slot is valid and is connected
	 * to a signal.
	 */
	void operator()(F... args)
	{
		if (isConnected && slot)
		{
			slot(args...);
		}
	}

	/**
	 * Checks if this item is connected.
	 */
	bool connected() const
	{
		return isConnected;
	}

	/**
	 * Detachs this item from the signal.
	 */
	void disconnect()
	{
		isConnected = false;
	}
};


/**
 * A Signal is a facility class for implementing the observer pattern more conveniently.
 * Signals are messages that can be received by others by registering a special function to the Singal. 
 * This special function is also called slot. Slot and signals are independent from each other but by connecting
 * A slot to a signal, the slot is called when the signal emits a message.
 */
template<class... F>
class Signal
{
public:
	typedef std::function<void(F...)> Slot;
	typedef SignalConnection<F...> Connection;
	typedef ScopedSignalConnection<F...> ScopedConnection;

private:
	typedef SignalConnectionItem<F...> ConnectionItem;
	typedef std::list<std::shared_ptr<ConnectionItem>> ConnectionList;

	ConnectionList list;

	/**
	 * Removes all disconnected slots.
	 */
	void clearDisconnected()
	{
		list.erase(std::remove_if(list.begin(), list.end(), [](std::shared_ptr<ConnectionItem>& item) {
			return !item->connected();
		}), list.end());
	}

public:

	Signal() {}

	~Signal()
	{
		for (auto& item : list)
		{
			item->disconnect();
		}
	}

	/**
	 * Emits a signal. The connected slots are called with the provided arguments args.
	 */
	void operator()(F... args)
	{
		std::list<std::shared_ptr<ConnectionItem>> list;
		for (auto& item : list)
		{
			if (item->connected())
			{
				list.push_back(item);
			}
		}

		for (auto& item : list)
		{
			(*item)(args...);
		}
			clearDisconnected();
	};

	/**
	 * Connects a slot to this signal.
	 * @return: The connection from the signal to the slot.
	 */
	Connection connect(const Slot& slot)
	{
		auto item = std::make_shared<ConnectionItem>(slot, true);
		list.push_back(item);
		return Connection(*this, item);
	}

	/**
	 * Detachs a connection from this signal.
	 * @return: true, if the connection could be found and deleted!
	 * false is returned, if the connection wasn't bound to this signal.
	 */
	bool disconnect(const Connection& connection)
	{
		bool found = false;
		for (auto& item : list)
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

	/**
	 * Detachs all connection to this signal.
	 */
	void disconnectAll()
	{
		for (auto& item : list)
		{
			item->disconnect();
		}
		clearDisconnected();
	}

	friend class Connecion;
};

/**
 * A signal connection represents a connection between a signal and a slot.
 */
template<class... F>
class SignalConnection
{
private:
	typedef SignalConnectionItem<F...> Item;

	Signal<F...>* signal;

	/**
	 * The connection item; holds the slot and information about
	 * the connection status.
	 */
	std::shared_ptr<Item> item;

public:
	SignalConnection()
		: signal(nullptr)
	{
	}

	/**
	 * Creates a new signal connection from a given signal and a given signal connection item.
	 */
	SignalConnection(Signal<F...>& signal, const std::shared_ptr<Item>& item)
		: signal(&signal), item(item)
	{
	}

	void operator=(const SignalConnection& other)
	{
		signal = other.signal;
		item = other.item;
	}

	virtual ~SignalConnection()
	{
	}

	/**
	 * Checks if the held signal connection item is equal to the provided one.
	 */
	bool hasItem(const Item& item) const
	{
		return item.get() == &item;
	}

	/**
	 * Is this signal connection connected?
	 */
	bool connected() const
	{
		return item->connected;
	}

	/**
	 * Disconnects this signal connection. 
	 * @return: true, if the connection was established before
	 * and could successfully be disconnected from the signal.
	 * False is returned, if the connection was already disconnected
	 * or isn't connected to the signal held by this signal connection.
	 */
	bool disconnect()
	{
		if (signal && item && item->connected())
		{
			return signal->disconnect(*this);
		}
		return false;
	}
};

/**
 * A scoped signal connection is a signal connection, that disconnects the held connection,
 * if it goes out of scope.
 */
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

	/**
	 * Disconnects the held connection before this object will be destroyed.
	 */
	~ScopedSignalConnection()
	{
		SignalConnection<F...>::disconnect();
	}

	/**
	 * Disconnects the current connection and assigns a new one to it.
	 */
	ScopedSignalConnection & operator=(const SignalConnection<F...>& connection)
	{
		SignalConnection<F...>::disconnect();
		SignalConnection<F...>::operator=(connection);
		return *this;
	}
};