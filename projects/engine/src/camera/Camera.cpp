#include <camera/Camera.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace glm;
using namespace platform;

Camera::Camera() : logClient(getLogServer(), true, true)
{
	fov = 45.0f;
	logClient.setPrefix("[Camera]");
}

Camera::Camera(vec3 position, vec3 look, vec3 up) :  fov(0), logClient(getLogServer())
{
	this->position = position;
	this->look = look;
	this->up = up;

	logClient.setPrefix("[Camera]");
}

Camera::Camera(const Camera& other) :  logClient(getLogServer())
{
	position = other.position;
	look = other.look;
	up = other.up;
	fov = other.fov;
	logClient.setPrefix("[Camera]");
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