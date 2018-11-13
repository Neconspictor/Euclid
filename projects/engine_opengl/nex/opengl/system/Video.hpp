#pragma once
#include <nex/system/System.hpp>
#include <nex/Window.hpp>

class RendererOpenGL;
class SystemUI;
class SubSystemProvider;

class Video : public System
{
public:
	Video(SubSystemProvider* system);

	virtual ~Video();
	void handle(Configuration& config) override;

	void init() override;

	void useRenderer(RendererOpenGL* renderer);

	SubSystemProvider* getWindowSystem() const;

	Window* getWindow() const;

private:
	std::string windowTitle;
	bool fullscreen;
	unsigned int width;
	unsigned int height;
	unsigned int colorBitDepth;
	unsigned int refreshRate;
	bool vSync;
	unsigned int msaaSamples;
	SubSystemProvider* windowSystem;
	Window* window;
	RendererOpenGL* renderer;
};
