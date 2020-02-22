#include <nex/common/Cache.hpp>




void foo() {
	nex::Cache<nex::FlexibleCacheItem<int>> cache;
	int id = 5;
	nex::flexible_ptr<int> value(new int(100), true);
	cache.insert(id, std::move(value));
	auto& cachedValue = cache.getCached(id);
	std::make_unique<int>();
}
