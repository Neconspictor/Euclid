#pragma once

#include <glm/glm.hpp>

namespace nex
{
	struct Sphere
	{
		glm::vec3 origin;
		float radius;

		/**
		 * Checks if a point lies on the sphere's hull
		 */
		bool liesOnHull(const glm::vec3& p) const;
	};
}