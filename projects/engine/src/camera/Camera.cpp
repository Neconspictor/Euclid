#include <camera/Camera.hpp>
#include <glm/glm.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace glm;
using namespace platform;

Camera::Camera() : logClient(getLogServer())
{
	yaw = pitch = 0;
	fov = 45.0f;
	logClient.add(makeConsoleEndpoint());
	logClient.setPrefix("[Camera]");
}

Camera::Camera(vec3 position, vec3 look, vec3 up) :  fov(0), logClient(getLogServer())
{
	this->position = position;
	this->look = look;
	this->up = up;

	yaw = pitch = 0;
	logClient.add(makeConsoleEndpoint());
	logClient.setPrefix("[Camera]");
}

Camera::Camera(const Camera& other) :  logClient(getLogServer())
{
	this->position = other.position;
	this->look = other.look;
	this->up = other.up;
	this->yaw = other.yaw;
	this->pitch = other.pitch;
	this->fov = other.fov;
	logClient.add(makeConsoleEndpoint());
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

const vec3 const& Camera::getUpDirection() const
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

void Camera::update(float mouseXFrameOffset, float mouseYFrameOffset)
{
	float sensitivity = 0.05f;
	mouseXFrameOffset *= sensitivity;
	mouseYFrameOffset *= sensitivity;

	yaw += mouseXFrameOffset;
	pitch += mouseYFrameOffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	vec3 front;
	front.x = sin(radians(yaw)) * cos(radians(pitch));
	front.y = sin(radians(pitch));
	front.z = -cos(radians(yaw)) * cos(radians(pitch));
	front = normalize(front);
	setLookDirection(front);
}

float Camera::getYaw() const
{
	return yaw;
}

float Camera::getPitch() const
{
	return pitch;
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