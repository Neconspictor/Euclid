#ifndef CAMERA_FPQUATERNION_CAMERA_HPP
#define CAMERA_FPQUATERNION_CAMERA_HPP
#include <camera/Camera.hpp>

class FPQuaternionCamera : public Camera
{
public:
	FPQuaternionCamera();
	FPQuaternionCamera(glm::vec3 position, glm::vec3 look, glm::vec3 up);
	FPQuaternionCamera(const FPQuaternionCamera& other);
	virtual ~FPQuaternionCamera();

	void rotateCamera(float Angle, float x, float y, float z);

	virtual void update(Input* input) override;

protected:
	float currentRotationX;
};

#endif