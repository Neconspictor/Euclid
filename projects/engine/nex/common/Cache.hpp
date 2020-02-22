#pragma once
#include <nex/util/Memory.hpp>
#include <unordered_map>
#include <nex/util/StringUtils.hpp>

namespace nex {

	template<class T>
	struct FlexibleCacheItem
	{
		using ID = int;
		using Value = nex::flexible_ptr<T>;
		using ValuePtr = T*;

		static ID createID(const std::filesystem::path& p) {
			return SID(p.generic_u8string());
		}
	};


	template<class Item>
	class Cache
	{
	public:
		using ID = typename Item::ID;
		using Value = typename Item::Value;
		using ValuePtr = typename Item::ValuePtr;

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