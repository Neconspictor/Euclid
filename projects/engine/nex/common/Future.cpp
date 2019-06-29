#include <nex/common/Future.hpp>
#include <future>

void nex::FutureTest()
{

	Promise<std::unique_ptr<int>> promise;

	auto future = promise.get_future();
	bool isReady = future.is_ready();

	promise.set_value(std::make_unique<int>(100));

	isReady = future.is_ready();
	auto result = future.get();

	std::future<bool> f;
	std::promise<bool> p;
	p.set_value(true);
}