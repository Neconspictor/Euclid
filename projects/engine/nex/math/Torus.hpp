#pragma once

#include <glm/glm.hpp>

namespace nex
{
	class Ray;

	struct Torus
	{
		struct RayIntersection {
			size_t intersectionCount = 0;
			float multipliers[4];
		};

		float innerRadius = 2.0f;
		float outerRadius = 0.5f;

		Torus(float innerRadius, float outerRadius);
		Torus();

		RayIntersection intersects(const Ray& ray);

		/**
		 * Checks if a point is located inside the torus topology.
		 */
		bool isInHull(const glm::vec3& p) const;
	};
}