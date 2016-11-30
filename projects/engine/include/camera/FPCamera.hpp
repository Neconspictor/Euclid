#ifndef CAMERA_FPCAMERA_HPP
#define CAMERA_FPCAMERA_HPP
#include <camera/FPCameraBase.hpp>

class FPCamera : public FPCameraBase
{
public:
	FPCamera();
	FPCamera(glm::vec3 position, glm::vec3 look, glm::vec3 up);
	FPCamera(const FPCamera& other);
	virtual ~FPCamera();

	virtual void update(Input* input, float frameTime) override;

	float getYaw() const;
	float getPitch() const;

protected:
	float yaw, pitch;
	float cameraSpeed;
};

#endif