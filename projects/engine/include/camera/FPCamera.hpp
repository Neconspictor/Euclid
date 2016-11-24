#ifndef CAMERA_FPCAMERA_HPP
#define CAMERA_FPCAMERA_HPP
#include <camera/Camera.hpp>

class FPCamera : public Camera
{
public:
	FPCamera();
	FPCamera(glm::vec3 position, glm::vec3 look, glm::vec3 up);
	FPCamera(const FPCamera& other);
	virtual ~FPCamera();

	virtual void update(int mouseXFrameOffset, int mouseYFrameOffset) override;

	float getYaw() const;
	float getPitch() const;

protected:
	float yaw, pitch;
};

#endif