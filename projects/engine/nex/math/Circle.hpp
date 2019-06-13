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

	private:
		Plane plane;

		// Note: the origin has to be located in the plane!
		glm::vec3 origin;
		float radius;
	};
}
