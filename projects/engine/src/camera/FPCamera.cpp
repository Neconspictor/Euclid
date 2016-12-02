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

void FPCamera::update(Input* input, float frameTime)
{
	float sensitivity = 0.05f;
	MouseOffset data = input->getFrameMouseOffset();
	float yawAddition = static_cast<float>(data.xOffset) * sensitivity;
	float pitchAddition = static_cast<float>(data.yOffset) * sensitivity;
	yaw += yawAddition;
	pitch += pitchAddition;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	vec3 front;
	front.x = sin(radians(yaw)) * cos(radians(pitch));
	front.y = sin(radians(-pitch));
	front.z = -cos(radians(yaw)) * cos(radians(pitch));
	front = normalize(front);
	setLookDirection(front);
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