#include <nex/config/VideoConfig.hpp>
#include <nex/config/Configuration.hpp>
#include <string>

using namespace std;

VideoConfig::VideoConfig() :
	fullscreen(false), width(0), height(0), colorBitDepth(0), refreshRate(0), 
	vSync(false), msaaSamples(0) {
}

void VideoConfig::handle(Configuration& config)
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

/*
void VideoConfig::init()
{

	if (!renderer)
	{
		throw_with_trace(runtime_error("Video::init(): No renderer was set!"));
	}

	Window::WindowStruct desc;
	desc.title = windowTitle;
	desc.fullscreen = fullscreen;
	desc.colorBitDepth = colorBitDepth;
	desc.refreshRate = refreshRate;
	desc.posX = 0;
	desc.posY = 0;
	desc.width = width;
	desc.height = height;
	desc.visible = true;
	desc.vSync = vSync;

	window = windowSystem->createWindow(desc);
	window->activate();

	renderer->setViewPort(0, 0, width, height);
	renderer->setMSAASamples(msaaSamples);
	renderer->init();
}*/