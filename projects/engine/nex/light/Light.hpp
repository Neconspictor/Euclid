#pragma once
#include <glm/glm.hpp>
#include "nex/math/Constant.hpp"

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

	class DirectionalLight
	{
	public:
		explicit DirectionalLight();

		const glm::vec3& getColor() const;

		const glm::vec3& getDirection() const;

		float getLightPower() const;

		void setColor(glm::vec3 color);

		void setDirection(glm::vec3 dir);

		void setPower(float power);

	protected:
		glm::vec3 mColor;
		glm::vec3 mDirection;
		float mPower;
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
