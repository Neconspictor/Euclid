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

	virtual const glm::vec3& getPosition() const;
	virtual const glm::vec3& getLookDirection() const;
	virtual const glm::vec3& getUpDirection() const;
	
	virtual void setPosition(const glm::vec3& position);
	virtual void setLookDirection(const glm::vec3& direction);
	virtual void setUpDirection(const glm::vec3& up);

	virtual void update(int mouseXFrameOffset, int mouseYFrameOffset);

	virtual float getFOV() const;

	virtual void onScroll(float yOffset);


protected:
	glm::vec3 position;
	glm::vec3 look;
	glm::vec3 up;
	float fov;
	platform::LoggingClient logClient;
private:
};

#endif