#pragma once

#include <nex/math/Plane.hpp>
#include <glm/glm.hpp>

namespace nex
{
	/**
	 * A Circle in 3D
	 */
	class Circle3D
	{
	public:
		Circle3D(Plane plane, glm::vec3 origin, float radius);

		const glm::vec3& getOrigin()const;
		const Plane& getPlane() const;
		float getRadius()const;

		/**
		 * Checks if a point is located on the circle.
		 * @param toleranceRange : points that aren't exactly on the circle, but are within this range, are treated to be on the circle.
		 */
		bool isOnCircle(const glm::vec3& point, float toleranceRange = 0.000001f) const;

	private:
		Plane plane;

		// Note: the origin has to be located in the plane!
		glm::vec3 origin;
		float radius;
	};
}