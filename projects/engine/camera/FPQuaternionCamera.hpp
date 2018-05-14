#pragma once
#include <camera/FPCameraBase.hpp>

class FPQuaternionCamera : public FPCameraBase
{
public:
	FPQuaternionCamera();
	FPQuaternionCamera(glm::vec3 position, glm::vec3 look, glm::vec3 up);
	FPQuaternionCamera(const FPQuaternionCamera& other);
	virtual ~FPQuaternionCamera();

	void rotateCamera(float Angle, float x, float y, float z);

	virtual void update(Input* input, float frameTime) override;

protected:
	float currentRotationX;
};