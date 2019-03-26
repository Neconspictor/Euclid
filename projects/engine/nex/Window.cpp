#include <nex/Window.hpp>
#include <imgui/imgui.h>

namespace nex
{

	Window::Window(WindowStruct const& description) :
		m_logger("Window")
	{
		width = description.width;
		height = description.height;
		colorBitDepth = description.colorBitDepth;
		fullscreen = description.fullscreen;
		refreshRate = description.refreshRate;
		posX = description.posX;
		posY = description.posY;
		title = description.title;
		m_isVisible = description.visible;
		m_isOpen = true;
		m_hasFocus = false;
		vSync = description.vSync;
	}

	void Window::setTitle(const std::string& newTitle)
	{
		title = newTitle;
	}

	void Window::setVsync(bool vsync)
	{
		this->vSync = vsync;
	}

	unsigned Window::getHeight() const
	{
		return height;
	}

	unsigned Window::getPosX() const
	{
		return posX;
	}

	unsigned Window::getPosY() const
	{
		return posY;
	}

	unsigned Window::getWidth() const
	{
		return width;
	}

	const std::string& Window::getTitle() const
	{
		return title;
	}

	bool Window::getVsync() const
	{
		return vSync;
	}

	bool Window::isInFullscreenMode()
	{
		return fullscreen;
	}

	Window_ConfigurationView::Window_ConfigurationView(Window* window) : m_window(window)
	{
	}

	void Window_ConfigurationView::drawSelf()
	{

		bool vsync = m_window->getVsync();

		// render configuration properties
		ImGui::PushID(m_id.c_str());

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