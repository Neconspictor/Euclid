#pragma once
#include <platform/Window.hpp>
#include <glm/detail/type_vec3.hpp>
#include <platform/logging/LoggingClient.hpp>
#include <glm/glm.hpp>


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
	
	virtual const glm::mat4& getView();

	virtual void setPosition(const glm::vec3& position);
	virtual void setLookDirection(const glm::vec3& direction);
	virtual void setUpDirection(const glm::vec3& up);

	virtual void update(Input* input, float frameTime);

	virtual void calcView();

	virtual float getFOV() const;

	virtual void onScroll(float yOffset);


protected:
	glm::vec3 position;
	glm::vec3 look;
	glm::vec3 up;
	glm::mat4 view;
	float fov;
	platform::LoggingClient logClient;
private:
};