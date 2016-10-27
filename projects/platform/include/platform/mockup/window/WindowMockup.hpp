#ifndef WINDOW_MOCKUP_HPP
#define WINDOW_MOCKUP_HPP
#include "platform/Window.hpp"

class WindowMockup : public Window
{
public:
	explicit WindowMockup(WindowStruct const& description);

	virtual ~WindowMockup(){}

	virtual void embedRenderer(Renderer* renderer) override;

	void setVisible(bool visible) override;

	void setFullscreen() override;
	
	void setWindowed() override;
	
	void resize(int newWidth, int newHeight) override;

	bool isOpen() override;

	void close() override;

	void pollEvents() override;
};
#endif