#pragma once
#include <glm/glm.hpp>
#include <nex/util/Projectional.hpp>
#include <nex/Input.hpp>

namespace nex
{
	class Camera : public Projectional
	{
	public:
		Camera();
		Camera(glm::vec3 position, glm::vec3 look, glm::vec3 up);
		Camera(const Camera& other);

		//virtual void onScroll(double xOffset, double yOffset);

		virtual void update(Input* input, float frameTime);
	};

}