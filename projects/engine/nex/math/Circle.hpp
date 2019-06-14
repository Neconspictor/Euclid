#pragma once

#include <nex/math/Plane.hpp>
#include <glm/glm.hpp>

namespace nex
{
	class Ray;

	/**
	 * A Circle in 3D
	 */
	class Circle3D
	{
	public:

		struct RayIntersection
		{
			// Specifies how much intersections with the circle exist.
			// Can be 0, 1 or 2
			unsigned intersectionCount = 0;

			// multipliers of the intersections
			float firstMultiplier = 0;
			float secondMultiplier = 0;
		};

		Circle3D(Plane plane, glm::vec3 origin, float radius);

		const glm::vec3& getOrigin()const;
		const Plane& getPlane() const;
		float getRadius()const;

		/**
		 * Checks if a ray intersects this circle
		 * @param toleranceRange : Points that aren't exactly on the circle, but are within this range, are accepted.
		 */
		RayIntersection intersects(const Ray& circle, float toleranceRange = 0.000001f) const;

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


	/**
	 * A min-max circle is a two-dimensional surface that is defined by a minimum radius and a maximum radius.
	 * All points that are located between these two radius are on the min-max circle.
	 */
	class MinMaxCircle3D
	{
	public:
		MinMaxCircle3D(Plane plane, glm::vec3 origin, float minRadius, float maxRadius);

		const glm::vec3& getOrigin()const;
		const Plane& getPlane() const;
		float getMaxRadius()const;
		float getMinRadius()const;
		

		/**
		 * Checks if a point is located on the min-max circle.
		 */
		bool onSurface(const glm::vec3& point) const;

	private:
		Plane mPlane;

		// Note: the origin has to be located in the plane!
		glm::vec3 mOrigin;
		float mMaxRadius;
		float mMinRadius;
	};
}