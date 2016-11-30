#include <camera/FPQuaternionCamera.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

FPQuaternionCamera::FPQuaternionCamera() : FPCameraBase(), currentRotationX(0)
{
	logClient.setPrefix("[FPCamera]");
}

FPQuaternionCamera::FPQuaternionCamera(vec3 position, vec3 look, vec3 up) : 
	FPCameraBase(position, look, up), currentRotationX(0)
{
	logClient.setPrefix("[FPCamera]");
}

FPQuaternionCamera::FPQuaternionCamera(const FPQuaternionCamera& other) : FPCameraBase(other), currentRotationX(0)
{
	logClient.setPrefix("[FPCamera]");
}

FPQuaternionCamera::~FPQuaternionCamera()
{
}

void FPQuaternionCamera::rotateCamera(float Angle, float x, float y, float z)
{
	quat temp, quat_view, result;

	temp.x = x * sin(Angle / 2);
	temp.y = y * sin(Angle / 2);
	temp.z = z * sin(Angle / 2);
	temp.w = cos(Angle / 2);

	quat_view.x = look.x;
	quat_view.y = look.y;
	quat_view.z = look.z;
	quat_view.w = 0;

	result = (temp * quat_view) * conjugate(temp);

	look.x = result.x;
	look.y = result.y;
	look.z = result.z;

	look = normalize(look);
}

void FPQuaternionCamera::update(Input* input, float frameTime)
{
	float sensitivity = 0.002f;
	MouseOffset data = input->getFrameMouseOffset();
	float rotateXAxis = data.yOffset * sensitivity;
	float rotateYAxis = data.xOffset * -sensitivity;
	currentRotationX += rotateXAxis;

	// We don't want to rotate up more than one radian, so we cap it.
	if (currentRotationX > 1)
	{
		currentRotationX = 1;
		rotateXAxis = 0;
	}
	// We don't want to rotate down more than one radian, so we cap it.
	if (currentRotationX < -1)
	{
		currentRotationX = -1;
		rotateXAxis = 0;
	}

	// get the axis to rotate around the x-axis. 
	vec3 axis = cross(look, up);
	// To be able to use the quaternion conjugate, the axis to
	// rotate around must be normalized.
	axis = normalize(axis);

	// Rotate around the y axis
	rotateCamera(rotateXAxis, axis.x, axis.y, axis.z);
	// Rotate around the x axis
	rotateCamera(rotateYAxis, up.x, up.y, up.z);

	doUserMovement(input, frameTime);
}