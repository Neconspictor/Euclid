#include <nex/common/Future.hpp>
#include <future>

void nex::FutureTest()
{

	auto func = std::function<void()>([]() {return; });

	std::packaged_task<void()> t(func);

	auto f = t.get_future();

	PackagedTask<int(bool)> packagedTask([=](bool value) {return 42; });

	auto packaged2 = std::move(packagedTask);
	auto packaged3(std::move(packaged2));

	auto futurePackaged = packaged3.get_future();
	auto futurePackaged2(std::move(futurePackaged));

	packaged3(false);

	std::cout << "result of packaged task = " << futurePackaged2.get() << std::endl;


	PackagedTask<void()> packagedTask2([=]{return ; });

	packagedTask2.get_future();
}