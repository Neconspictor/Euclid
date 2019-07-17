#pragma once
#include "Constant.hpp"
#include <glm/glm.hpp>

namespace nex
{
	class Ray;

	/**
	 * Represents the plane equation defined by a signed distance to the origin and a normal vector.
	 */
	struct Plane
	{
		struct RayIntersection
		{
			// The multiplier for the intersection point.
			long double multiplier = 0.0f;

			// minimal one intersection
			bool intersected = false;

			// is parallel to the plane (i.d. either infinite solutions (intersected true) or none at all (intersected false))
			bool parallel = false;
		};

		glm::vec3 mNormal = { 0,0,-1 };
		float mSignedDistance = 0;

		Plane();
		Plane(glm::vec3 normal, float distance);

		/**
		 * Creates a plane from a normal and a point laying on the desired plane.
		 */
		Plane(glm::vec3 normal, glm::vec3 pointOnPlane);

		Plane(float x, float y, float z, float d);

		/**
		 * Checks if a ray intersects the plane
		 */
		RayIntersection intersects(const Ray& ray) const;

		/**
		 * Tests if a vector (direction or point) is on the plane
		 */
		bool onPlane(const glm::vec3& point) const;

		/**
		 * Projects a point orthogonally on the plane.
		 */
		glm::vec3 project(const glm::vec3& point) const;
	};

	Plane normalize(const Plane& plane);

	/**
	 * Transforms a plane by a transformation matrix
	 */
	Plane operator*(const glm::mat4& trafo, const Plane& plane);

	/**
	 * 
	 */
	Plane transformWithTransposeInverse(const glm::mat4& transposeInverseTrafo, const Plane& plane);
}
