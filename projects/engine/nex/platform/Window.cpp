#include <nex/platform/Window.hpp>
#include <imgui/imgui.h>

namespace nex
{

	Window::Window(WindowStruct const& description) :
		mLogger("Window"),
		mHasFocus(false),
		mConfig(description)
	{
		
	}

	void Window::setTitle(const std::string& newTitle)
	{
		mConfig.title = newTitle;
	}

	void Window::setVsync(bool vsync)
	{
		mConfig.vSync = vsync;
	}

	unsigned Window::getFrameBufferHeight() const
	{
		return mConfig.frameBufferHeight;
	}

	unsigned Window::getPosX() const
	{
		return mConfig.posX;
	}

	unsigned Window::getPosY() const
	{
		return mConfig.posY;
	}

	unsigned Window::getFrameBufferWidth() const
	{
		return mConfig.frameBufferWidth;
	}

	unsigned Window::getVirtualScreenHeight() const
	{
		return mConfig.virtualScreenHeight;
	}

	unsigned Window::getVirtualScreenWidth() const
	{
		return mConfig.virtualScreenWidth;
	}

	const std::string& Window::getTitle() const
	{
		return mConfig.title;
	}

	bool Window::getVsync() const
	{
		return mConfig.vSync;
	}

	bool Window::isInFullscreenMode() const
	{
		return mConfig.fullscreen;
	}

	Window_ConfigurationView::Window_ConfigurationView(Window* window) : m_window(window)
	{
	}

	void Window_ConfigurationView::drawSelf()
	{

		bool vsync = m_window->getVsync();

		// render configuration properties
		ImGui::PushID(mId.c_str());

		if (ImGui::Checkbox("Vertical Synchronization", &vsync))
		{
			if (vsync != m_window->getVsync())
			{
				m_window->setVsync(vsync);
			}
		}
		if (ImGui::Button("Toggle Fullscreen/Windowed mode"))
		{
			if (m_window->isInFullscreenMode())
			{
				m_window->setWindowed();
			}
			else
			{
				m_window->setFullscreen();
			}
		}

		/*char buffer[256];

		std::string title = m_window->getTitle();

		title._Copy_s(buffer, 256, title.size());

		if (ImGui::InputText("Title", buffer, sizeof(buffer)))
		{
			if (m_window->getTitle() != buffer)
			{
				m_window->setTitle(buffer);
			}
		}*/

		ImGui::PopID();
	}
}