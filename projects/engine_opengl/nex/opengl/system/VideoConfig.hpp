#pragma once
#include <nex/system/System.hpp>
#include <nex/Window.hpp>

class RendererOpenGL;
class SystemUI;
class SubSystemProvider;

struct VideoConfig
{
public:
	VideoConfig();

	void handle(Configuration& config);

	std::string windowTitle;
	bool fullscreen;
	unsigned int width;
	unsigned int height;
	unsigned int colorBitDepth;
	unsigned int refreshRate;
	bool vSync;
	unsigned int msaaSamples;
};
