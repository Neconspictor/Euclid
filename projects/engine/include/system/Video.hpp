#pragma once
#include <system/System.hpp>
#include <platform/Window.hpp>
#include <renderer/Renderer3D.hpp>

class SystemUI;
class WindowSystem;

class Video : public System
{
public:
	Video(WindowSystem* system);

	virtual ~Video();
	void handle(const CollectOptions& config) override;

	virtual void init() override;

	void useRenderer(Renderer3D* renderer);

	void useUISystem(SystemUI* ui);

	WindowSystem* getWindowSystem() const;

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
	WindowSystem* windowSystem;
	Window* window;
	Renderer3D* renderer;
	SystemUI* ui;

};
