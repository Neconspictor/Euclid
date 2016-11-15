#include <platform/Platform.hpp>
#include <sstream>
#include <boost/current_function.hpp>
#include <boost/predef.h>

using namespace std;

// include platform dependent stuff!
#if BOOST_OS_WINDOWS 
#include <platform/windows/PlatformWindows.hpp>

shared_ptr<Platform> Platform::singleton = 
	make_shared<PlatformWindows> (PlatformWindows());
#endif

/*shared_ptr<Window> PlatformFactory::createWindow(const Window::WindowStruct& desc)
{
#ifdef PLATFORM_WINDOWS 
	return make_shared(new WindowWin32(desc));
#else 
	stringstream ss;
	ss << BOOST_CURRENT_FUNCTION << ", EXCEPTION: Current platform not supported!";
	throw runtime_error(ss.str());
#endif
}*/

shared_ptr<Platform> Platform::getActivePlatform()
{

#if BOOST_OS_WINDOWS
	return singleton;
#else
	stringstream ss;
	ss << BOOST_CURRENT_FUNCTION << ", EXCEPTION: Current platform not supported!";
	throw runtime_error(ss.str());
#endif
}