#pragma once

class Configuration;

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
