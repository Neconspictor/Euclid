#include <nex/math/Ray.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/math/Math.hpp>
#include <nex/common/Log.hpp>

nex::Ray::Ray(const glm::vec3& origin, const glm::vec3& dir): origin(origin), dir(normalize(dir))
{
	if (glm::length(dir) == 0.0f) throw_with_trace(std::invalid_argument("nex::Ray : length of direction vector mustn't be 0!"));

	// Note: It is ok if division by zero occurs!
	invDir = 1.0f / dir;

	sign.x = invDir.x < 0;
	sign.y = invDir.y < 0;
	sign.z = invDir.z < 0;
}

nex::Ray::PointDistance nex::Ray::calcClosestDistance(const glm::vec3& point) const
{
	//Reference for math: http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html

	// Get a point on the ray. We use the one with t= 1 so that the difference between the origin and the point
	// produces a unit vector.
	const glm::vec3 pointOnLine = getPoint(1.0f);

	// create two direction vectors
	const glm::vec3 fromOrigin = point - origin;
	const glm::vec3 fromPointOnLine = point - pointOnLine;

	// Note that fromPointOnLine - origin is a unit vector -> we don't need to divide by its length!
	const float distance = length(cross(fromOrigin, fromPointOnLine));

	// get the angle between the vector from origin to point and the ray direction.
	// This angle can be used to get the projected point on the ray and thus is the multiplier we are looking for.
	const float angle = dot(fromOrigin, dir);
	//const glm::vec3 projectedPoint = origin + angle * dir;

	// Check that the calculated projected point has indeed the previously calculated distance to the point.
	auto cmp = distance - length(getPoint(angle) - point);
	if (cmp > 0.002f) {
		LOG(Logger("nex::Ray::calcClosestDistance"), Error) << "Projected point has not the expected distance to the reference point!";
	}

	return { angle, distance };
}

nex::Ray::RayDistance nex::Ray::calcClosestDistance(const Ray& ray) const
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

glm::vec3 nex::Ray::project(const glm::vec3& p) const
{
	// Note: v mustn't be normalized! 
	// We need its length to calc the distance to the point and the projected point.
	glm::vec3 v = p - origin;
	const auto vLen = length(v);
	v = normalize(v);
	constexpr auto eps = 0.000001f;

	//Check if p already is on the ray
	if (length(v - dir) < eps)
		return p;

	// get orthogonal vector 
	// Note: It doesn't matter in which way we do the cross product for normal and w.
	// We fix the direction with the sign of dot(v,w)!
	const auto normal = normalize(cross(dir, v));

	// w will be a vector that is on the same direction as (p-p') or (p'-p)
	const auto w = normalize(cross(dir, normal));

	// compute distance to p'
	// smallest angle between v and w
	const auto vDotW = dot(v, w);

	// Note: v points from origin to p. Thus if w points to p as well, the angle between v and w will be < 90° => vDotW is positive.
	// In order to get from p to p' we have to use -w => use -vDotW.
	// For the other case (w points to p') the angle between v and w will be > 90° => vDotW is negative.
	// In order to get from p to p' we have to use w => use -vDotW.
	// So, in both cases we need -vDotW!
	const auto signedDistance = -vDotW * vLen;

	return p + signedDistance * w;
}