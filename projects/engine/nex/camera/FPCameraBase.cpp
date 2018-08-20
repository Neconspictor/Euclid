#include <nex/camera/FPCameraBase.hpp>

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
	vec3 cameraUp = normalize(cross(cameraRight, look));

	if (input->isDown(Input::KEY_W))
		position += moveAmount * look;

	if (input->isDown(Input::KEY_S))
		position -= moveAmount * look;

	if (input->isDown(Input::KEY_D))
		position += moveAmount * cameraRight;

	if (input->isDown(Input::KEY_A))
		position -= moveAmount * cameraRight;

	if (input->isDown(Input::KEY_Z))
		position += moveAmount * cameraUp;

	if (input->isDown(Input::KEY_X))
		position -= moveAmount * cameraUp;
}