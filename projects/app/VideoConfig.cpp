#include <VideoConfig.hpp>
#include <nex/config/Configuration.hpp>
#include <string>

using namespace std;

nex::VideoConfig::VideoConfig() :
	fullscreen(false), width(0), height(0), colorBitDepth(0), refreshRate(0), 
	vSync(false), msaaSamples(0) {
}

void nex::VideoConfig::handle(Configuration& config)
{
	config.addOption("Video", "Title", &windowTitle, string(""));
	config.addOption("Video", "Fullscreen", &fullscreen, false);
	config.addOption<unsigned int>("Video", "WindowWidth", &width, 800);
	config.addOption<unsigned int>("Video", "WindowHeight", &height, 600);
	config.addOption<unsigned int>("Video", "ColorBitDepth", &colorBitDepth, 32);
	config.addOption<unsigned int>("Video", "RefreshRate", &refreshRate, 60);
	config.addOption("Video", "VSync", &vSync, false);
	config.addOption<unsigned int>("Video", "MSAASamples", &msaaSamples, 0);
}