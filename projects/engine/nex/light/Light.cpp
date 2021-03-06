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
		 * As we use the y-axis as the zenith direction, we have to rotate the coordinates -90� around the x-axis.
		 * The same conversion can be done, if we replace z with -y and y with z
		 * This results in this formula: 
		 *  x = radius * sin(polar) * cos(azimuth),
		 * 	y = radius * cos(polar)
		 *	z = -radius * sin(polar) * sin(azimuth)
		 */

		return glm::vec3(
			-coord.radius * sin(coord.polar) * cos(coord.azimuth), //x
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
		 *  azimuth = arctan(y / x)
		 * 	
		 * For zenith direction = (0,1,0) we replace z with -y and y with z
		 * This results to:
		 *  radius = length(cartesian)
		 *  polar = arccos(y / radius)
		 *  azimuth = arctan(-z / x)
		 */

		result.radius = length(cartesian);
		result.polar = glm::acos(cartesian.y / result.radius);

		// Note: atan handles infinity properly (if x is 0)
		result.azimuth = glm::atan(cartesian.z / cartesian.x);

		return result;
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
}