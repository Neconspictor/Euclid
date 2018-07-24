#pragma once
#include <platform/Window.hpp>
#include <glm/glm.hpp>
#include <util/Projectional.hpp>


class Camera : public Projectional
{
public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 look, glm::vec3 up);
	Camera(const Camera& other);
	virtual ~Camera();

	virtual void onScroll(double xOffset, double yOffset);

	virtual void update(Input* input, float frameTime);
};
