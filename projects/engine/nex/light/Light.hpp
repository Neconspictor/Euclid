#pragma once
#include <glm/glm.hpp>
#include "nex/math/Constant.hpp"
#include <interface/light_interface.h>

namespace nex {

	/**
	 * Spherical coordintes with a zenith direction of (0,1,0) (the y-axis)
	 */
	struct SphericalCoordinate
	{
		float polar;
		float azimuth;
		float radius;

		static glm::vec3 cartesian(const SphericalCoordinate& coord);
		static SphericalCoordinate convert(glm::vec3 cartesian);
	};

	class AmbientLight
	{
	public:
		AmbientLight();
		const glm::vec3& getColor()const;
		float getPower() const;
		void setColor(glm::vec3 color);
		void setPower(float power);
		

	private:
		glm::vec3 mColor;
		float mPower;
	};
}