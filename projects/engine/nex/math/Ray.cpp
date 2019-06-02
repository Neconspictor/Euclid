#include <nex/math/Ray.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/math/Math.hpp>

nex::Ray::Ray(const glm::vec3& origin, const glm::vec3& dir): origin(origin), dir(normalize(dir))
{
	if (glm::length(dir) == 0.0f) throw_with_trace(std::invalid_argument("nex::Ray : length of direction vector mustn't be 0!"));

	// Note: It is ok if division by zero occurs!
	invDir = 1.0f / dir;

	sign.x = invDir.x < 0;
	sign.y = invDir.y < 0;
	sign.z = invDir.z < 0;
}

const glm::vec3& nex::Ray::getDir() const
{
	return dir;
}

const glm::vec3& nex::Ray::getInvDir() const
{
	return invDir;
}

const glm::vec3& nex::Ray::getOrigin() const
{
	return origin;
}

glm::vec3 nex::Ray::getPoint(float multiplier) const
{
	return origin + multiplier * dir;
}

const glm::uvec3& nex::Ray::getSign() const
{
	return sign;
}