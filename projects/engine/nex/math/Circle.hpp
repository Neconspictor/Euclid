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
		Circle3D(Plane plane, glm::vec3 origin, Real radius);

		const glm::vec3& getOrigin()const;
		const Plane& getPlane() const;
		Real getRadius()const;

		/**
		 * Checks if a point is located on the circle.
		 * @param toleranceRange : points that aren't exactly on the circle, but are within this range, are treated to be on the circle.
		 */
		bool isOnCircle(const glm::vec3& point, Real toleranceRange = 0.000001f) const;

	private:
		Plane plane;

		// Note: the origin has to be located in the plane!
		glm::vec3 origin;
		Real radius;
	};


	/**
	 * A min-max circle is a two-dimensional surface that is defined by a minimum radius and a maximum radius.
	 * All points that are located between these two radius are on the min-max circle.
	 */
	class MinMaxCircle3D
	{
	public:
		MinMaxCircle3D(Plane plane, glm::vec3 origin, Real minRadius, Real maxRadius);

		const glm::vec3& getOrigin()const;
		const Plane& getPlane() const;
		Real getMaxRadius()const;
		Real getMinRadius()const;
		

		/**
		 * Checks if a point is located on the min-max circle.
		 */
		bool onSurface(const glm::vec3& point) const;

	private:
		Plane mPlane;

		// Note: the origin has to be located in the plane!
		glm::vec3 mOrigin;
		Real mMaxRadius;
		Real mMinRadius;
	};
}