#include <system/Video.hpp>
#include <system/Configuration.hpp>
#include <string>
#include <platform/Window.hpp>
#include <platform/WindowSystem.hpp>
#include <platform/Platform.hpp>

using namespace std;

Video::Video(WindowSystem* system) :
	System("Video"), fullscreen(false), width(0), height(0), colorBitDepth(0), refreshRate(0), vSync(false),
	windowSystem(system), window(nullptr), renderer(nullptr)
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

	if (!renderer)
	{
		throw runtime_error("Video::init(): No renderer was set!");
	}

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
	desc.visible = true;
	desc.vSync = vSync;

	//window = Platform::getActivePlatform()->createWindow(desc, *renderer);
	window = windowSystem->createWindow(desc, *renderer);
	window->activate();
	renderer->setViewPort(0, 0, width, height);
	renderer->init();
	//Platform::getActivePlatform()->setVSync(*renderer, vSync);
}

void Video::useRenderer(Renderer* renderer)
{
	this->renderer = renderer;
}

WindowSystem* Video::getWindowSystem() const
{
	return windowSystem;
}

Window* Video::getWindow() const
{
	return window;
}
