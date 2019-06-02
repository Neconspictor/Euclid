#pragma once

namespace nex
{
	/**
	 * Represents the plane equation defined by a signed distance to the origin and a normal vector.
	 */
	struct Plane
	{
		glm::vec3 normal = { 0,0,-1 };
		float signedDistance = 0;

		Plane() {}
		Plane(glm::vec3 normal, float distance) : normal(normal), signedDistance(distance) {}
		Plane(float x, float y, float z, float d)
		{
			normal = { x,y,z };
			signedDistance = d;
		}
	};

	Plane normalize(const Plane& plane);

	/**
	 * Transforms a plane by a transformation matrix
	 */
	Plane operator*(const glm::mat4& trafo, const Plane& plane);
	Plane transform(const glm::mat4& trafo, const Plane& plane);
}