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

	virtual void calcView();

	float getFarPlaneDistance() const;
	
	virtual float getFOV() const;
	
	float getHDimension() const;
	
	virtual const glm::vec3& getLookDirection() const;
	
	float getNearPlaneDistance() const;
	
	const glm::mat4& Camera::getOrthogonalProjection();
	
	const glm::mat4& Camera::getPerspectiveProjection();
	
	virtual const glm::vec3& getPosition() const;
	
	virtual const glm::vec3& getUpDirection() const;
	
	float getVDimension() const;
	
	virtual const glm::mat4& getView();

	virtual void onScroll(double xOffset, double yOffset);

	void setFarPlaneDistance(float farPlaneDistance);

	void setFOV(float fov);

	void setHDimension(float dimension);

	virtual void setLookDirection(const glm::vec3& direction);
	
	void setNearPlaneDistance(float nearPlaneDistance);
	
	virtual void setPosition(const glm::vec3& position);
	
	virtual void setUpDirection(const glm::vec3& up);

	virtual void update(Input* input, float frameTime);
	
	void setVDimension(float dimension);

protected:
	float farPlaneDistance;
	float fov;
	float hDimension;
	glm::vec3 look;
	float nearPlaneDistance;
	platform::LoggingClient logClient;
	glm::mat4 orthoProjection;
	glm::vec3 position;
	glm::mat4 perspProjection;
	bool revalidate;
	glm::vec3 up;
	float vDimension;
	glm::mat4 view;

	virtual void update();
};