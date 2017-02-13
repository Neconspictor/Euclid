#include <camera/FPCamera.hpp>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

FPCamera::FPCamera() : FPCameraBase(), yaw(0), pitch(0), cameraSpeed(5.0f)
{
	logClient.setPrefix("[FPCamera]");
}

FPCamera::FPCamera(vec3 position, vec3 look, vec3 up) : FPCameraBase(position, look, up),
	yaw(0), pitch(0), cameraSpeed(5.0f)
{
	logClient.setPrefix("[FPCamera]");
}

FPCamera::FPCamera(const FPCamera& other) : FPCameraBase(other)
{
	yaw = other.yaw;
	pitch = other.pitch;
	cameraSpeed = other.cameraSpeed;
	logClient.setPrefix("[FPCamera]");
}

FPCamera::~FPCamera()
{
}

float FPCamera::limit(float source, float minValue, float maxValue)
{
	if (source > maxValue)
		source = maxValue;
	if (source < minValue)
		source = minValue;
	return source;
}

void FPCamera::setLook(vec3 direction)
{
	// it is assumed that look is a normalized vector -> important for arcsinus!
	Camera::setLook(direction);

	yaw = degrees(atan2(look.x, -look.z));

	// look.y = sin(radians(-pitch));
	// <-> -degrees(asin(look.y)) = pitch
	pitch = degrees(asin(-look.y));
	pitch = limit(pitch, -89.0f, 89.0f);
}

void FPCamera::update(Input* input, float frameTime)
{
	float sensitivity = 0.05f;
	MouseOffset data = input->getFrameMouseOffset();
	float yawAddition = static_cast<float>(data.xOffset) * sensitivity;
	float pitchAddition = static_cast<float>(data.yOffset) * sensitivity;
	yaw += yawAddition;
	pitch += pitchAddition;

	pitch = limit(pitch, -89.0f, 89.0f);

	vec3 front;
	front.x = sin(radians(yaw)) * cos(radians(pitch));
	front.y = sin(radians(-pitch));
	front.z = -cos(radians(yaw)) * cos(radians(pitch));
	front = normalize(front);
	//look = normalize(front);
	setLook(front);
	//Camera::setLookDirection(direction);
	doUserMovement(input, frameTime);
}

float FPCamera::getYaw() const
{
	return yaw;
}

float FPCamera::getPitch() const
{
	return pitch;
}