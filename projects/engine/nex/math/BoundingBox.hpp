#pragma once

#include <glm/glm.hpp>
#include <nex/math/Constant.hpp>

namespace nex
{
	class Ray;
	struct AABB2;

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
			float firstIntersection;

			/**
			 * Multiplier of the second intersection point.
			 */
			float secondIntersection;
		};

		glm::vec3 min;
		glm::vec3 max;

		/**
		 * Checks if a given ray intersects this AABB bounding box.
		 */
		RayIntersection testRayIntersection(const nex::Ray& ray) const;

		AABB();
		AABB(const glm::vec3& min, const glm::vec3& max);
		AABB(glm::vec3&& min, glm::vec3&& max);

		/**
		 * Constructs an AABB from an AABB2
		 */
		explicit AABB(const AABB2&);
		AABB& operator= (const AABB2&);
	};

	/**
	 * An alternative representation of an AABB using a center point and a half width vector.
	 */
	struct AABB2 
	{
		glm::vec3 center;
		glm::vec3 halfWidth;

		AABB2();
		AABB2(const glm::vec3& center, const glm::vec3& halfWidth);
		AABB2(glm::vec3&& center, glm::vec3&& halfWidth);

		/**
		 * Constructs an AABB2 from an AABB
		 */
		explicit AABB2(const AABB&);
		AABB2& operator= (const AABB&);
	};

	AABB maxAABB(const AABB& a, const AABB& b);

	AABB operator*(const glm::mat4& trafo, const AABB& box);
}