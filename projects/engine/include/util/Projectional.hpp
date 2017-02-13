#pragma once

#include <glm/glm.hpp>
#include <platform/logging/LoggingClient.hpp>

struct Frustum
{
	float left;
	float right;
	float bottom;
	float top;
	float nearPlane;
	float farPlane;
};

enum ProjectionMode
{
	Orthographic,
	Perspective
};

class Projectional
{
public:

	explicit Projectional(float aspectRatio = 16.0f/9.0f, 
		float fov = 45.0f, 
		Frustum frustum = {-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 150.0f},
		//Frustum frustum = Frustum(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 150.0f),
		glm::vec3 look = { 0,0,-1 },
		glm::vec3 position = {0,0,0},
		glm::vec3 up = {0,1,0}
	);

	virtual ~Projectional();

	virtual void calcView();
	float getAspectRatio() const;
	const glm::vec3& getLook() const;
	float getFOV() const;
	const Frustum& getFrustum() const;
	const glm::mat4& getOrthoProjection();
	const glm::mat4& getPerspProjection();
	const glm::mat4& getProjection(ProjectionMode mode);
	const glm::vec3& getPosition() const;
	const glm::vec3& getUp() const;
	const glm::mat4& getView();

	void lookAt(glm::vec3 location);

	/**
	 * NOTE: ratio as to be greater 0, otherwise a runtime_error will be thrown!
	 */
	void setAspectRatio(float ratio);

	void setFOV(float fov);

	void setFrustum(Frustum frustum);

	/**
	* NOTE: Has to be a vector that isn't a null vector. So it's length has to be > 0
	*/
	virtual void setLook(glm::vec3 look);

	virtual void setPosition(glm::vec3 position);

	virtual void setUp(glm::vec3 up);

protected:
	float aspectRatio;
	float fov;
	Frustum frustum;
	platform::LoggingClient logClient;
	glm::vec3 look;
	glm::mat4 orthographic;
	glm::vec3 position;
	glm::mat4 perspective;
	bool revalidate;
	glm::vec3 up;
	glm::mat4 view;

	virtual void update();
};