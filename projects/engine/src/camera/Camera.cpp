#include <camera/Camera.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace glm;
using namespace platform;

Camera::Camera(Window* window) : position(0,0,0), look(0,0,-1), up(0,1,0), fov(45.0f), logClient(getLogServer(), true, true),
	window(window)
{
	logClient.setPrefix("[Camera]");
	if (!window) throw std::runtime_error("Camera::Camera(Window*): window isn't set!");
}

Camera::Camera(Window* window, vec3 position, vec3 look, vec3 up) :  Camera(window)
{
	this->position = position;
	this->look = look;
	this->up = up;
}

Camera::Camera(const Camera& other) :  Camera(other.window)
{
	position = other.position;
	look = other.look;
	up = other.up;
	fov = other.fov;
	if (!window) throw std::runtime_error("Camera::Camera(const Camera&): window isn't set!");
}

Camera::~Camera()
{
}

const vec3& Camera::getPosition() const
{
	return position;
}

const vec3& Camera::getLookDirection() const
{
	return look;
}

const vec3& Camera::getUpDirection() const
{
	return up;
}

const mat4& Camera::getView()
{
	return view;
}

void Camera::setActiveWindow(Window* window)
{
	if (!window) throw std::runtime_error("Camera::setActiveWindow(Window*): window is null!");
	this->window = window;
}

void Camera::setPosition(const vec3& position)
{
	this->position = position;
}

void Camera::setLookDirection(const vec3& direction)
{
	this->look = direction;
}

void Camera::setUpDirection(const vec3& up)
{
	this->up = up;
}

void Camera::update(int mouseXFrameOffset, int mouseYFrameOffset)
{
}

void Camera::calcView()
{
	view = lookAt(position, position + look, up);
}

float Camera::getFOV() const
{
	return fov;
}

void Camera::onScroll(float yOffset)
{
	// zoom
	LOG(logClient, Debug) << "scrollYFrameOffset: " << yOffset;
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yOffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}