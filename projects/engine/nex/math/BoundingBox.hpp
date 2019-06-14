#pragma once

#include <glm/glm.hpp>
#include <nex/math/Constant.hpp>

namespace nex
{
	class Ray;

	struct AABB
	{
		struct RayIntersection
		{
			/**
			 * Specifies if the ray intersected the bounding box.
			 */
			bool intersected;

			/**
			 * Multiplier of the first intersection point.
			 */
			Real firstIntersection;

			/**
			 * Multiplier of the second intersection point.
			 */
			Real secondIntersection;
		};

		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);

		/**
		 * Checks if a given ray intersects this AABB bounding box.
		 */
		RayIntersection testRayIntersection(const nex::Ray& ray) const;
	};

	AABB operator*(const glm::mat4& trafo, const AABB& box);
}