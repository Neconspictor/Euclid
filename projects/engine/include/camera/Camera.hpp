#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <glm/detail/type_vec3.hpp>
#include "listener/ScrollListener.hpp"

class Camera : public ScrollListener
{
public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 look, glm::vec3 up);
	Camera(const Camera& other);
	virtual ~Camera();

	const glm::vec3& getPosition() const;
	const glm::vec3& getLookDirection() const;
	const glm::vec3& getUpDirection() const;
	
	void setPosition(const glm::vec3& position);
	void setLookDirection(const glm::vec3& direction);
	void setUpDirection(const glm::vec3& up);

	void update(float mouseXFrameOffset, float mouseYFrameOffset);

	float getYaw();
	float getPitch();
	float getFOV();

	virtual void onScroll(float yOffset);


protected:
private:
	glm::vec3 position;
	glm::vec3 look;
	glm::vec3 up;
	float yaw, pitch;
	float fov;
};

#endif