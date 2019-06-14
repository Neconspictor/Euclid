#pragma once

#include <glm/glm.hpp>

namespace nex
{
	struct Torus
	{
		glm::vec3 origin = glm::vec3(0.0f);
		float radius = 1.0f;

		Torus(const glm::vec3& origin, float radius);
		Torus();

		/**
		 * Checks if a point lies on the sphere's hull
		 */
		bool liesOnHull(const glm::vec3& p) const;
	};
}