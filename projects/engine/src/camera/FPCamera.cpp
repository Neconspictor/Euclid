#include <camera/FPCamera.hpp>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

FPCamera::FPCamera() : Camera(), yaw(0), pitch(0)
{
	logClient.setPrefix("[FPCamera]");
}

FPCamera::FPCamera(vec3 position, vec3 look, vec3 up) : Camera(position, look, up), 
	yaw(0), pitch(0)
{
	logClient.setPrefix("[FPCamera]");
}

FPCamera::FPCamera(const FPCamera& other) : Camera(other)
{
	yaw = other.yaw;
	pitch = other.pitch;
	logClient.setPrefix("[FPCamera]");
}

FPCamera::~FPCamera()
{
}

void FPCamera::update(int mouseXFrameOffset, int mouseYFrameOffset)
{
	float sensitivity = 0.05f;
	mouseXFrameOffset = static_cast<int>(mouseXFrameOffset * sensitivity);
	mouseYFrameOffset = static_cast<int>(mouseYFrameOffset * sensitivity);

	yaw += mouseXFrameOffset;
	pitch += mouseYFrameOffset;

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
}

float FPCamera::getYaw() const
{
	return yaw;
}

float FPCamera::getPitch() const
{
	return pitch;
}