#pragma once
#include <nex/camera/FPCameraBase.hpp>

class FPCamera : public FPCameraBase
{
public:
	FPCamera();
	FPCamera(glm::vec3 position, glm::vec3 look, glm::vec3 up);
	FPCamera(const FPCamera& other);
	virtual ~FPCamera();


	float limit(float source, float min, float max);
	virtual void setLook(glm::vec3 direction) override;
	virtual void update(Input* input, float frameTime) override;

	float getYaw() const;
	float getPitch() const;

protected:
	float yaw, pitch;
	float cameraSpeed;
};