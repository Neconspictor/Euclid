#include "camera/Camera.hpp"
#include <glm/glm.hpp>
#include <iostream>

using namespace glm;

Camera::Camera() : ScrollListener()
{
	yaw = pitch = 0;
	fov = 45.0f;
}

Camera::Camera(glm::vec3 position, glm::vec3 look, glm::vec3 up) : ScrollListener()
{
	this->position = position;
	this->look = look;
	this->up = up;

	yaw = pitch = 0;

}

Camera::Camera(const Camera& other) : ScrollListener()
{
	this->position = other.position;
	this->look = other.look;
	this->up = other.up;
	this->yaw = other.yaw;
	this->pitch = other.pitch;
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

void Camera::setPosition(const glm::vec3& position)
{
	this->position = position;
}

void Camera::setLookDirection(const glm::vec3& direction)
{
	this->look = direction;
}

void Camera::setUpDirection(const glm::vec3& up)
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

float Camera::getYaw()
{
	return yaw;
}

float Camera::getPitch()
{
	return pitch;
}

float Camera::getFOV()
{
	return fov;
}

void Camera::onScroll(float yOffset)
{
	// zoom
	std::cout << "scrollYFrameOffset: " << yOffset << std::endl;
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yOffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}
