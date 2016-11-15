#ifndef NEC_ENGINE_VIDEO_HPP
#define NEC_ENGINE_VIDEO_HPP
#include <system/System.hpp>
#include <platform/Window.hpp>

class Video : public System
{
public:
	Video();

	virtual ~Video();
	void handle(const CollectOptions& config) override;

	virtual void init() override;

	void useRenderer(std::shared_ptr<Renderer> renderer);

	std::shared_ptr<Window> getWindow();

private:
	std::string windowTitle;
	bool fullscreen;
	unsigned int colorBitDepth;
	unsigned int refreshRate;
	bool vSync;
	std:: shared_ptr<Window> window;
	std::shared_ptr<Renderer> renderer;

};

#endif