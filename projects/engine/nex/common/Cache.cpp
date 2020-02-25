#include <nex/common/Cache.hpp>

#include <unordered_set>
#include <nex/scene/Vob.hpp>


void foo() {
	nex::Cache<nex::FlexibleCacheItem<int>> cache;
	int id = 5;
	nex::flexible_ptr<int> value = std::make_unique<int>(66);
	cache.insert(id, std::move(value));
	auto& cachedValue = cache.getCached(id);
	std::make_unique<int>();

	nex::flexible_ptr<int> value2(new int(1), true);

	std::unordered_set<nex::flexible_ptr<nex::Vob>> set;

	auto v = std::make_unique<nex::Vob>();
	set.insert(std::move(v));



}
