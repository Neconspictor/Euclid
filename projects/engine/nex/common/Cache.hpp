#pragma once
#include <nex/util/Memory.hpp>
#include <unordered_map>
#include <nex/util/StringUtils.hpp>

namespace nex {

	template<class T, class Key = unsigned>
	struct FlexibleCacheItem
	{
		using ID = Key;
		using Value = nex::flexible_ptr<T>;
		using ValuePtr = T*;

		static ValuePtr getValuePtr(Value& t) {
			return t.get();
		}
		
	};


	template<class Item>
	class Cache
	{
	public:
		using ID = typename Item::ID;
		using Value = typename Item::Value;
		using ValuePtr = typename Item::ValuePtr;
		//using getValuePtr = typename Item::getValuePtr;

		void clear() {
			mCachedItems.clear();
		}


		bool isCached(const ID& id) const {
			return mCachedItems.find(id) != mCachedItems.end();
		}

		/**
		 * Provides the value of a specified id.
		 * If the id doesn't refer to a previously inserted value, nullptr will be returned.
		 */
		ValuePtr getCachedPtr(const ID& id) {
			auto it = mCachedItems.find(id);
			if (mCachedItems.find(id) == mCachedItems.end()) return nullptr;
			return Item::getValuePtr((*it).second);
		}

		/**
		 * Provides the value of a specified id.
		 * @throws std::out_of_range exception if id wasn't inserted previously.
		 */
		Value& getCached(const ID& id) {
			return mCachedItems.at(id);
		}

		void insert(const ID& id, Value value) {
			mCachedItems.insert(std::make_pair<>(id, std::move(value)));
		}

	private:
		std::unordered_map<ID, Value> mCachedItems;
	};

}