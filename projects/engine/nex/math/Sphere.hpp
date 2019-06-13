#pragma once

#include <glm/glm.hpp>

namespace nex
{
	struct Sphere
	{
		glm::vec3 origin = glm::vec3(0.0f);
		float radius = 1.0f;

		Sphere(const glm::vec3& origin, float radius);
		Sphere();

		/**
		 * Checks if a point lies on the sphere's hull
		 */
		bool liesOnHull(const glm::vec3& p) const;
	};
}