#ifndef ENGINE_TEST_ENGINE_TESTER_HPP
#define ENGINE_TEST_ENGINE_TESTER_HPP
#include <platform/event/TaskManager.hpp>
#include <system/System.hpp>

class TestSystem : public System {
public:
	TestSystem(const std::weak_ptr<platform::LoggingServer>& server) :
		System("TestSystem", server)
	{
		enableUpdater();
		mSomeConfigSetting = 25;
		addSetting("SomeConfigSetting", &mSomeConfigSetting);
	}

	virtual ~TestSystem() {
	}

	virtual void update() {
		std::cout << "." << mSomeConfigSetting++;

		if (mSomeConfigSetting > 30)
			channel.broadcast(TaskManager::StopEvent());
	}

	unsigned int mSomeConfigSetting;
};

void testEngine();
#endif