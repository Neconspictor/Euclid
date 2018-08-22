#include <nex/Window.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>

Window::Window(WindowStruct const& description):
logClient(nex::getLogServer())
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

	logClient.setPrefix("[Window]");
}

void Window::setTitle(const std::string& newTitle)
{
	title = newTitle;
}

void Window::setVsync(bool vsync)
{
	this->vSync = vsync;
}

int Window::getHeight() const
{
	return height;
}

int Window::getPosX() const
{
	return posX;
}

int Window::getPosY() const
{
	return posY;
}

int Window::getWidth() const
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
	/*ImGui::DragFloat("yaw", &m_camera->yaw, 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat("pitch", &m_camera->pitch, 1.0f, -89.0f, 89.0f);
	ImGui::DragFloat("fov", &m_camera->fov, 1.0f, 0.0f, 90.0f);
	ImGui::DragFloat("aspect ratio", &m_camera->aspectRatio, 0.1f, 0.1f, 90.0f);
	ImGui::DragFloat("near plane", &m_camera->perspFrustum.nearPlane, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat("far plane", &m_camera->perspFrustum.farPlane, 1.0f, 1.0f, 10000.0f);
	ImGui::DragFloat("speed", &m_camera->cameraSpeed, 0.2f, 0.0f, 100.0f);*/

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
		} else
		{
			m_window->setFullscreen();
		}
	}

	ImGui::PopID();
}
