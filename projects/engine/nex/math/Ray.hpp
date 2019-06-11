#pragma once

namespace nex
{
	struct Plane;

	/**
	 * A 3-dimensional ray specified by a starting point (origin) and a direction.
	 */
	class Ray
	{
	public:

		struct RayRayDistance
		{
			float multiplier;
			float otherMultiplier;
			bool parallel;
			float distance;
		};

		struct RayPointDistance
		{
			float multiplier; //multiplier for the projection of the point on the line
			float distance;
		};

		struct RayPlaneIntersection
		{
			// The multiplier for the intersection point.
			float multiplier = 0.0f;

			// minimal one intersection
			bool intersected = false;

			// is parallel to the plane (i.d. either infinite solutions (intersected true) or none at all (intersected false))
			bool parallel = false; 
		};

		/**
		 * Constructs a new ray from a starting point (origin) and a direction.
		 * Note: length of direction vector must be != 0.
		 * @throws std::invalid_argument : if length of the direction vector is 0.
		 */
		Ray(const glm::vec3& origin, const glm::vec3& dir);

		/**
		 * Calculates the closest (perpendicular) signed distance to a point.
		 */
		RayPointDistance calcClosestDistance(const glm::vec3& point) const;

		/**
		 * Calculates the closest (perpendicular) signed distance to another ray.
		 */
		RayRayDistance calcClosestDistance(const Ray& ray) const;

		const glm::vec3& getDir() const;

		/**
		 * Provides the inverse of the direction vector.
		 * NOTE: some components can be INF or -INF if the direction vector has a component that is 0.
		 * But as the length of the direction vector mustn't be 0, not all components of the inverse will be INF or -INF!
		 */
		const glm::vec3& getInvDir() const;

		const glm::vec3& getOrigin() const;

		glm::vec3 getPoint(float multiplier) const;

		const glm::uvec3& getSign() const;

		/**
		 * Checks if this ray intersects a plane
		 */
		RayPlaneIntersection intersects(const Plane& plane) const;

	private:
		glm::vec3 origin;
		glm::vec3 dir;

		/**
		 * Specifies the inverse of the direction.
		 * Note: infinity and minus infinity are clamped to [-FLT_MAX, FLT_MAX]
		 */
		glm::vec3 invDir;

		/**
		 * Specifies where the inverse direction is smaller 0 (0) or greater/equal zero (1)
		 */
		glm::uvec3 sign;
	};
}