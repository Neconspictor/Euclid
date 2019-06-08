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

nex::Ray::RayRayDistance nex::Ray::calcClosestDistance(const Ray& ray) const
{
	const auto dirSquared = dot(dir, dir);
	const auto rayDirSquared = dot(ray.dir, ray.dir);
	const auto dirDotRayDir = dot(dir, ray.dir);
	constexpr float eps = 0.00001f;

	const auto determinant = dirSquared * rayDirSquared - dirDotRayDir * dirDotRayDir;

	// if the rays are parallel we use the perpendicular distance between this ray and the origin of ray
	const bool parallel = determinant < eps;
	if (parallel)
	{
		const auto p1p2 = origin - ray.origin;
		const float distance = length(p1p2 - dot(p1p2, dir)*dir);

		return { 0.0f, 0.0f, parallel, distance };
	}

	// not parallel

	const auto invDeterminant = 1.0f / determinant;
	const glm::mat2 invMat = invDeterminant * glm::mat2( rayDirSquared, dirDotRayDir, // first column
													dirDotRayDir,  dirSquared // second column
												 );
	const glm::vec2 vec(dot(dir, ray.origin - origin), dot(ray.dir, origin - ray.origin));
	const glm::vec2 multipliers = invMat * vec;
	const float distance = length(ray.origin + multipliers.y * ray.dir - origin - multipliers.x * dir);

	return { multipliers.x, multipliers.y, parallel, distance };
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