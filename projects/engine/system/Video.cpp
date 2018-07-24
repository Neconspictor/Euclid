#include <system/Video.hpp>
#include <system/Configuration.hpp>
#include <string>
#include <platform/Window.hpp>
#include <platform/PlatformProvider.hpp>

using namespace std;

Video::Video(SubSystemProvider* system) :
	System("Video"), fullscreen(false), width(0), height(0), colorBitDepth(0), refreshRate(0), 
	vSync(false), msaaSamples(0), windowSystem(system), window(nullptr), renderer(nullptr)
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
	config.config->addOption<unsigned int>(getName(), "WindowWidth", &width, 800);
	config.config->addOption<unsigned int>(getName(), "WindowHeight", &height, 600);
	config.config->addOption<unsigned int>(getName(), "ColorBitDepth", &colorBitDepth, 32);
	config.config->addOption<unsigned int>(getName(), "RefreshRate", &refreshRate, 60);
	config.config->addOption(getName(), "VSync", &vSync, false);
	config.config->addOption<unsigned int>(getName(), "MSAASamples", &msaaSamples, 0);
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

	window = windowSystem->createWindow(desc);
	window->activate();

	renderer->setViewPort(0, 0, width, height);
	renderer->setMSAASamples(msaaSamples);
	renderer->init();
}

void Video::useRenderer(Renderer3D* renderer)
{
	this->renderer = renderer;
}

SubSystemProvider* Video::getWindowSystem() const
{
	return windowSystem;
}

Window* Video::getWindow() const
{
	return window;
}