#pragma once

#include <glm/glm.hpp>

namespace nex
{
	struct Torus
	{
		float innerRadius = 2.0f;
		float outerRadius = 0.5f;

		Torus(float innerRadius, float outerRadius);
		Torus();

		/**
		 * Checks if a point is located inside the torus topology.
		 */
		bool isInHull(const glm::vec3& p) const;
	};
}