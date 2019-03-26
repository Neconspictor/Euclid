#include <nex/camera/FPCameraBase.hpp>

using namespace glm;

nex::FPCameraBase::FPCameraBase() : Camera({0,0,0}, {0,0,-1}, {0,1,0}), cameraSpeed(5.0f)
{
}

nex::FPCameraBase::FPCameraBase(vec3 position, vec3 look, vec3 up) : Camera(position, look, up),
	cameraSpeed(5.0f)
{
}

nex::FPCameraBase::FPCameraBase(const FPCameraBase& other) : Camera(other),
	cameraSpeed(other.cameraSpeed)
{
}

void nex::FPCameraBase::doUserMovement(Input* input, float frameTime)
{
	// camera movements
	float moveAmount = cameraSpeed * frameTime;
	vec3 cameraRight = normalize(cross(look, up));
	vec3 cameraUp = normalize(cross(cameraRight, look));

	vec3 direction(0.0f);

	/*if (input->isDown(Input::KEY_W))
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
		position -= moveAmount * cameraUp;*/

	if (input->isDown(Input::KEY_W))
		direction += look;

	if (input->isDown(Input::KEY_S))
		direction -= look;

	if (input->isDown(Input::KEY_D))
		direction += cameraRight;

	if (input->isDown(Input::KEY_A))
		direction -= cameraRight;

	if (input->isDown(Input::KEY_Z))
		direction += cameraUp;

	if (input->isDown(Input::KEY_X))
		direction -= cameraUp;

	if (length(direction) > 0)
		position += moveAmount * normalize(direction);
}