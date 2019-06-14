#pragma once
#include "Constant.hpp"

namespace nex
{
	/**
	 * Represents the plane equation defined by a signed distance to the origin and a normal vector.
	 */
	struct Plane
	{
		glm::vec3 mNormal = { 0,0,-1 };
		Real mSignedDistance = 0;

		Plane();
		Plane(glm::vec3 normal, Real distance);

		/**
		 * Creates a plane from a normal and a point laying on the desired plane.
		 */
		Plane(glm::vec3 normal, glm::vec3 pointOnPlane);

		Plane(Real x, Real y, Real z, Real d);

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
	Plane transform(const glm::mat4& trafo, const Plane& plane);
}
