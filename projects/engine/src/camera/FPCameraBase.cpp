#include <camera/FPCameraBase.hpp>

using namespace glm;

FPCameraBase::FPCameraBase() : Camera({0,0,0}, {0,0,-1}, {0,1,0}), cameraSpeed(5.0f)
{
}

FPCameraBase::FPCameraBase(vec3 position, vec3 look, vec3 up) : Camera(position, look, up), 
	cameraSpeed(5.0f)
{
}

FPCameraBase::FPCameraBase(const FPCameraBase& other) : Camera(other), 
	cameraSpeed(other.cameraSpeed)
{
}

FPCameraBase::~FPCameraBase()
{
}

void FPCameraBase::doUserMovement(Input* input, float frameTime)
{
	// camera movements
	float moveAmount = cameraSpeed * frameTime;
	vec3 cameraRight = normalize(cross(look, up));

	if (input->isDown(Input::KeyW))
		position += moveAmount * look;

	if (input->isDown(Input::KeyS))
		position -= moveAmount * look;

	if (input->isDown(Input::KeyD))
		position += moveAmount * cameraRight;

	if (input->isDown(Input::KeyA))
		position -= moveAmount * cameraRight;
}