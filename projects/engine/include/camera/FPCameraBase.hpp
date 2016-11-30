#ifndef CAMERA_FPCAMERABASE_HPP
#define CAMERA_FPCAMERABASE_HPP
#include <camera/Camera.hpp>

class FPCameraBase : public Camera
{
public:
	FPCameraBase();
	FPCameraBase(glm::vec3 position, glm::vec3 look, glm::vec3 up);
	FPCameraBase(const FPCameraBase& other);
	~FPCameraBase() override;
	void doUserMovement(Input* input, float frameTime);

protected:
	float cameraSpeed;
};

#endif