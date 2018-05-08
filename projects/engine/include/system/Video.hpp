#pragma once
#include <system/System.hpp>
#include <platform/Window.hpp>
#include <renderer/Renderer3D.hpp>

class SystemUI;
class PlatformProvider;

class Video : public System
{
public:
	Video(PlatformProvider* system);

	virtual ~Video();
	void handle(const CollectOptions& config) override;

	virtual void init() override;

	void useRenderer(Renderer3D* renderer);

	void useUISystem(SystemUI* ui);

	PlatformProvider* getWindowSystem() const;

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
	PlatformProvider* windowSystem;
	Window* window;
	Renderer3D* renderer;
	SystemUI* ui;

};
