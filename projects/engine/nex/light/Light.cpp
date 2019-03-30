#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/light/Light.hpp>

//#include <glm/gtc/matrix_transform.inl>

using namespace std;
//using namespace glm;

namespace nex {
	glm::vec3 SphericalCoordinate::cartesian(const SphericalCoordinate& coord)
	{
		/**
		 *
		 * The formula with a zenith direction of (0,0,1) (z-axis) is:
		 *  x = radius * sin(polar) * cos(azimuth),
		 * 	y = radius * sin(polar) * sin(azimuth),
		 *	z = radius * cos(polar)
		 *	
		 *	for more info: https://en.wikipedia.org/wiki/List_of_common_coordinate_transformations#To_spherical_coordinates
		 * 
		 * As we use the y-axis as the zenith direction, we have to rotate the coordinates -90° around the x-axis.
		 * The same conversion can be done, if we replace z with -y and y with z
		 * This results in this formula: 
		 *  x = radius * sin(polar) * cos(azimuth),
		 * 	y = radius * cos(polar)
		 *	z = -radius * sin(polar) * sin(azimuth)
		 */

		return glm::vec3(
			coord.radius * sin(coord.polar) * cos(coord.azimuth), //x
			coord.radius * cos(coord.polar), //y
			-coord.radius * sin(coord.polar) * sin(coord.azimuth) //z
		);
	}

	SphericalCoordinate SphericalCoordinate::convert(glm::vec3 cartesian)
	{
		SphericalCoordinate result;

		/**
		 * The formula with a zenith direction of (0,0,1) (z-axis) is:
		 *  radius = length(cartesian)
		 *  polar = arccos(z / radius)
		 *  azimuth = arctan(y / z)
		 * 	
		 * For zenith direction = (0,1,0) we replace z with -y and y with z
		 * This results to:
		 *  radius = length(cartesian)
		 *  polar = arccos(-y / radius)
		 *  azimuth = arctan(z / -y)
		 */

		result.radius = length(cartesian);
		result.polar = glm::acos(-cartesian.y / result.radius);
		result.azimuth = glm::atan(cartesian.z / -cartesian.y);

		return result;
	}

	DirectionalLight::DirectionalLight() : mColor(1, 1, 1), mDirection(1,1,1), mPower(1.0f)
	{
	}

	const glm::vec3& DirectionalLight::getColor() const
	{
		return mColor;
	}

	const glm::vec3& DirectionalLight::getDirection() const
	{
		return mDirection;
	}

	float DirectionalLight::getLightPower() const
	{
		return mPower;
	}

	void DirectionalLight::setPower(float power)
	{
		mPower = power;
	}

	void DirectionalLight::setColor(glm::vec3 color)
	{
		this->mColor = move(color);
	}

	void DirectionalLight::setDirection(glm::vec3 dir)
	{
		mDirection = move(dir);
	}

	AmbientLight::AmbientLight() : mColor(1.0f), mPower(1.0f)
	{
	}

	const glm::vec3& AmbientLight::getColor() const
	{
		return mColor;
	}

	float AmbientLight::getPower() const
	{
		return mPower;
	}

	void AmbientLight::setColor(glm::vec3 color)
	{
		mColor = std::move(color);
	}

	void AmbientLight::setPower(float power)
	{
		mPower = power;
	}

	PointLight::PointLight() : Projectional()
	{
		fov = 90.0f;
		aspectRatio = 1.0f;
		perspFrustum.nearPlane = 0.1f;
		perspFrustum.farPlane = 1.0f;
		m_logger.setPrefix("PointLight");
	}

	glm::mat4* PointLight::getMatrices()
	{
		update();
		return shadowMatrices;
	}

	float PointLight::getRange() const
	{
		return perspFrustum.farPlane;
	}

	void PointLight::setRange(float range)
	{
		perspFrustum.farPlane = range;
		revalidate = true;
	}

	void PointLight::update(bool alwaysUpdate)
	{
		if (revalidate || alwaysUpdate)
		{
			glm::vec3 test2 = mCurrentPosition;
			glm::vec3 test = glm::vec3(1.0, 0.0, 0.0);
			test2 + test;

			glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspectRatio, perspFrustum.nearPlane, perspFrustum.farPlane);
			// right plane
			shadowMatrices[0] = shadowProj * glm::lookAt(mCurrentPosition, mCurrentPosition + glm::vec3(1.0, 0.0, 0.0),
				glm::vec3(0.0, -1.0, 0.0));
			// left plane
			shadowMatrices[1] = shadowProj * glm::lookAt(mCurrentPosition, mCurrentPosition + glm::vec3(-1.0, 0.0, 0.0),
				glm::vec3(0.0, -1.0, 0.0));
			// top plane
			shadowMatrices[2] = shadowProj * glm::lookAt(mCurrentPosition, mCurrentPosition + glm::vec3(0.0, 1.0, 0.0),
				glm::vec3(0.0, 0.0, 1.0));
			// bottom plane
			shadowMatrices[3] = shadowProj * glm::lookAt(mCurrentPosition, mCurrentPosition + glm::vec3(0.0, -1.0, 0.0),
				glm::vec3(0.0, 0.0, -1.0));
			// front plane
			shadowMatrices[4] = shadowProj * glm::lookAt(mCurrentPosition, mCurrentPosition + glm::vec3(0.0, 0.0, 1.0),
				glm::vec3(0.0, -1.0, 0.0));
			// back plane
			shadowMatrices[5] = shadowProj * glm::lookAt(mCurrentPosition, mCurrentPosition + glm::vec3(0.0, 0.0, -1.0),
				glm::vec3(0.0, -1.0, 0.0));
		}
		Projectional::update();
	}

}