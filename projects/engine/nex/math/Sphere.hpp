#pragma once

#include <glm/glm.hpp>

namespace nex
{
	class Ray;
	struct AABB;

	struct Sphere
	{

		struct RayIntersection
		{
			// Specifies how much intersections with the sphere exist.
			// Can be 0, 1 or 2
			unsigned intersectionCount = 0;

			// multipliers of the intersections
			float firstMultiplier = 0;
			float secondMultiplier = 0;
		};

		glm::vec3 origin = glm::vec3(0.0f);
		float radius = 1.0f;

		Sphere(const glm::vec3& origin, float radius);
		Sphere();

		/**
		 * Checks if a ray intersects the circle
		 */
		RayIntersection intersects(const Ray& ray) const;

		bool intersects(const AABB& box) const;

		/**
		 * Checks if a point lies on the sphere's hull
		 */
		bool isInHull(const glm::vec3& p) const;
	};
}