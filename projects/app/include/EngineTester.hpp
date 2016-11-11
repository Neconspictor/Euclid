#ifndef ENGINE_TEST_ENGINE_TESTER_HPP
#define ENGINE_TEST_ENGINE_TESTER_HPP
#include <platform/event/TaskManager.hpp>
#include <system/System.hpp>
#include <system/Configuration.hpp>

class TestSystem : public System {
public:

	TestSystem(const std::weak_ptr<platform::LoggingServer>& server) :
		System("TestSystem", server)
	{
		enableUpdater();
		mSomeConfigSetting = 25;
		mStringConfigSetting = "Hello World!";
		mAnotherStringConfigSetting = "Test!";
	}

	virtual ~TestSystem() {
	}

	virtual void update() {
		LOG(logClient, platform::Debug) << "." << mSomeConfigSetting++;

		if (mSomeConfigSetting > 30)
			channel.broadcast(TaskManager::StopEvent());
	}


	void handle(const CollectOptions& config) override {
		//addSetting(config.config, "SomeConfigSetting", &mSomeConfigSetting, mSomeConfigSetting);
		LOG(logClient, platform::Debug) << "handle(const CollectOptions& config) called for " << getName();
		config.config->addOption(getName(), "SomeConfigSetting", &mSomeConfigSetting, mSomeConfigSetting);
		config.config->addOption(getName(), "StringConfigSetting", &mStringConfigSetting, mStringConfigSetting);
		config.config->addOption(getName(), "AnotherStringConfigSetting", &mAnotherStringConfigSetting, mAnotherStringConfigSetting);
	};

	double mSomeConfigSetting;
	std::string mStringConfigSetting;
	std::string mAnotherStringConfigSetting;
};

void testEngine();
#endif