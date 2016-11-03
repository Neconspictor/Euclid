#include <TaskManagementTester.hpp>
#include <iostream>

std::atomic<size_t> completeCount;

void testTaskManagement() {
	TaskManager tm;

	// try queue with no multithreaded tasks
	tm.add(TaskManager::TaskPtr(new EndTask(&tm)));
	tm.start();

	// try a bunch of background tasks and one that waits for everything to be complete
	std::atomic_store<size_t>(&completeCount, 0);
	completeCount = 0;
	for (int i = 0; i < 10; ++i) {
		tm.add(TaskManager::TaskPtr(new FakeMultithreaded(65536)));
		tm.add(TaskManager::TaskPtr(new FakeMultithreaded(4096)));
		tm.add(TaskManager::TaskPtr(new FakeMultithreaded(1000)));
		tm.add(TaskManager::TaskPtr(new FakeMultithreaded(2)));
		tm.add(TaskManager::TaskPtr(new FakeMultithreaded(1024*1024*100)));
	}
	tm.add(TaskManager::TaskPtr(new CheckedStop(&tm)));

	tm.start();
}