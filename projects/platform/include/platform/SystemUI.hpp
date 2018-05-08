#pragma once

class Window;
class PlatformProvider;

class SystemUI
{
public:
	static SystemUI* get(PlatformProvider* windowSystem);

	static void shutdown();

	virtual void init(Window* window) = 0;

	virtual void frameUpdate() = 0;


protected:
	SystemUI();
	virtual ~SystemUI();
private:
	static SystemUI* instance;
};
