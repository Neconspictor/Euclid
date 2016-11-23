#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <glm/detail/type_vec3.hpp>
#include <platform/logging/LoggingClient.hpp>

class Camera
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

	void update(int mouseXFrameOffset, int mouseYFrameOffset);

	float getYaw() const;
	float getPitch() const;
	float getFOV() const;

	void onScroll(float yOffset);


protected:
private:
	glm::vec3 position;
	glm::vec3 look;
	glm::vec3 up;
	float yaw, pitch;
	float fov;
	platform::LoggingClient logClient;
};

#endif