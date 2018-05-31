#pragma once
#include <system/System.hpp>
#include <platform/Window.hpp>
#include <renderer/Renderer3D.hpp>

class SystemUI;
class SubSystemProvider;

class Video : public System
{
public:
	Video(SubSystemProvider* system);

	virtual ~Video();
	void handle(const CollectOptions& config) override;

	virtual void init() override;

	void useRenderer(Renderer3D* renderer);

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
	Renderer3D* renderer;
};
