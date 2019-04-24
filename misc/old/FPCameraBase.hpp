#pragma once
#include <nex/camera/Camera.hpp>

namespace nex
{
	class FPCameraBase : public Camera
	{
	public:
		FPCameraBase();
		FPCameraBase(glm::vec3 position, glm::vec3 look, glm::vec3 up);
		FPCameraBase(const FPCameraBase& other);
		void doUserMovement(Input* input, float frameTime);

	protected:
		float cameraSpeed;
	};
}