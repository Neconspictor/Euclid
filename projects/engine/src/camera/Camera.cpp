#include <camera/Camera.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace glm;
using namespace platform;

Camera::Camera() : farPlaneDistance(10.0f), fov(45.0f), hDimension(10.0f), look(0,0,-1), 
nearPlaneDistance(1.0f), logClient(getLogServer(), true, true), position(0,0,0), revalidate(true), 
up(0,1,0), vDimension(10.0f)
{
	logClient.setPrefix("[Camera]");
}

Camera::Camera(vec3 position, vec3 look, vec3 up) :  Camera()
{
	this->position = position;
	this->look = look;
	this->up = up;
}

Camera::Camera(const Camera& other) :  Camera()
{
	position = other.position;
	look = other.look;
	up = other.up;
	fov = other.fov;
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

float Camera::getNearPlaneDistance() const
{
	return nearPlaneDistance;
}

const vec3& Camera::getUpDirection() const
{
	return up;
}

float Camera::getVDimension() const
{
	return vDimension;
}

const mat4& Camera::getView()
{
	return view;
}

void Camera::setPosition(const vec3& position)
{
	this->position = position;
}

void Camera::setLookDirection(const vec3& direction)
{
	this->look = direction;
}

void Camera::setNearPlaneDistance(float nearPlaneDistance)
{
	this->nearPlaneDistance = nearPlaneDistance;
	revalidate = true;
}

void Camera::setUpDirection(const vec3& up)
{
	this->up = up;
}

void Camera::setVDimension(float dimension)
{
	vDimension = dimension;
	revalidate = true;
}

void Camera::update()
{
	// only update if changes have occurred
	if (revalidate)
	{
		//orthoProjection = ortho(-hDimension * 0.5f, hDimension * 0.5f, -vDimension * 0.5f, 
		//	vDimension * 0.5f, nearPlaneDistance, farPlaneDistance);
		orthoProjection = ortho(-fov, fov, -fov,
			fov, nearPlaneDistance, farPlaneDistance);
		perspProjection = perspective(radians(fov),
			hDimension / vDimension, nearPlaneDistance, farPlaneDistance);
		revalidate = false;
	}
}

void Camera::update(Input* input, float frameTime)
{
}

const mat4& Camera::getOrthogonalProjection()
{
	update();
	return orthoProjection;
}

const mat4& Camera::getPerspectiveProjection()
{
	update();
	return perspProjection;
}

void Camera::calcView()
{
	view = lookAt(position, position + look, up);
}

float Camera::getFarPlaneDistance() const
{
	return farPlaneDistance;
}

float Camera::getFOV() const
{
	return fov;
}

float Camera::getHDimension() const
{
	return hDimension;
}

void Camera::onScroll(double xOffset, double yOffset)
{
	// zoom
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yOffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;

	revalidate = true;
}

void Camera::setFarPlaneDistance(float farPlaneDistance)
{
	this->farPlaneDistance = farPlaneDistance;
	revalidate = true;
}

void Camera::setFOV(float fov)
{
	this->fov = fov;
	revalidate = true;
}

void Camera::setHDimension(float dimension)
{
	hDimension = dimension;
	revalidate = true;
}