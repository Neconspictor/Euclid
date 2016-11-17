#include <system/Video.hpp>
#include <system/Configuration.hpp>
#include <string>
#include <platform/Window.hpp>
#include <platform/Platform.hpp>

using namespace std;

Video::Video() :
	System("Video"), fullscreen(false), width(0), height(0), colorBitDepth(0), refreshRate(0), vSync(false)
{
	//we dont't need a frame updater
	updater.reset();
	assert(updater.get() == nullptr);
}

Video::~Video()
{
}

void Video::handle(const CollectOptions& config)
{
	config.config->addOption(getName(), "Title", &windowTitle, string(""));
	config.config->addOption(getName(), "Fullscreen", &fullscreen, false);
	config.config->addOption(getName(), "WindowWidth", &width, unsigned int(800));
	config.config->addOption(getName(), "WindowHeight", &height, unsigned int(600));
	config.config->addOption(getName(), "ColorBitDepth", &colorBitDepth, unsigned int(32));
	config.config->addOption(getName(), "RefreshRate", &refreshRate, unsigned int(60));
	config.config->addOption(getName(), "VSync", &vSync, false);
}

void Video::init()
{
	System::init();

	Window::WindowStruct desc;
	desc.title = windowTitle;
	desc.fullscreen = fullscreen;
	desc.colorBitDepth = colorBitDepth;
	desc.refreshRate = refreshRate;
	desc.posX = 0;
	desc.posY = 0;
	desc.width = width;
	desc.height = height;
	desc.visible = false;

	window = Platform::getActivePlatform()->createWindow(desc);

	if (renderer)
	{
		window->embedRenderer(renderer);
		Platform::getActivePlatform()->setVSync(*renderer.get(), vSync);
	}

	window->setVisible(true);
}

void Video::useRenderer(shared_ptr<Renderer> renderer)
{
	this->renderer = renderer;
	if (window)
	{
		window->embedRenderer(renderer);
		Platform::getActivePlatform()->setVSync(*renderer.get(), vSync);
	}
}

shared_ptr<Window> Video::getWindow()
{
	return window;
}