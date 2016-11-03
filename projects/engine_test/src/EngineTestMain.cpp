#include <EngineTester.hpp>
#include <system/Engine.hpp>
#include <iostream>

using namespace std;

int main()
{
	shared_ptr<TestSystem> ts(new TestSystem());
	Engine engine;

	engine.add(ts);

	cout << "Running" << endl;
	engine.run();
	cout << "Done" << endl;
	//testEngine();
	return EXIT_SUCCESS;
}