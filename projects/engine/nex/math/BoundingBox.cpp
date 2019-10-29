#include <nex/math/BoundingBox.hpp>
#include <nex/math/Math.hpp>
#include <nex/math/Ray.hpp>

nex::AABB::RayIntersection nex::AABB::testRayIntersection(const nex::Ray& ray) const
{
	const glm::vec3* bounds[2] = {&min, &max};
	const auto& sign = ray.getSign();
	const auto& origin = ray.getOrigin();
	const auto& invDir = ray.getInvDir();

	/**
	 * Note: the inverse direction can have components that are INF or -INF.
	 * INF or -INF multiplied by 0 will result into a NaN.
	 * The following algorithm handles NaNs correctly as comparisons between NaN and any valid float value
	 * will always be false.
	 */

	float tmin = -FLT_MAX;
	float tmax = FLT_MAX;

	const float tminX = (bounds[sign.x]->x - origin.x) * invDir.x;
	const float tmaxX = (bounds[1 - sign.x]->x - origin.x) * invDir.x;

	if (tminX > tmin) tmin = tminX;
	if (tmaxX < tmax) tmax = tmaxX;

	const float tminY = (bounds[sign.y]->y - origin.y) * invDir.y;
	const float tmaxY = (bounds[1 - sign.y]->y - origin.y) * invDir.y;

	if ((tmin > tmaxY) || (tminY > tmax)) return {false, 0, 0};

	if (tminY > tmin) tmin = tminY;
	if (tmaxY < tmax) tmax = tmaxY;

	const float tminZ = (bounds[sign.z]->z - origin.z) * invDir.z;
	const float tmaxZ = (bounds[1 - sign.z]->z - origin.z) * invDir.z;

	if ((tmin > tmaxZ) || (tminZ > tmax)) return {false, 0, 0};

	if (tminZ > tmin) tmin = tminZ;
	if (tmaxZ < tmax) tmax = tmaxZ;

	/**
	 * If the ray direction would be allowed to be the null vector (-> all components are 0)
	 * we would have to test this here and change tmin and tmax to 0 
	 * (-> bounding box is a point and lies at the origin of the ray).
	 * But as a ray direction is guaranteed to be not the null vector, this additional test is unnecessary
	 * and can be skipped.
	 */

	return {true, tmin, tmax};
}

nex::AABB::AABB() : min(glm::vec3(std::numeric_limits<float>::max())), max(glm::vec3(-std::numeric_limits<float>::max()))
{
};

nex::AABB::AABB(const glm::vec3& min, const glm::vec3& max)
{
	this->min = min;
	this->max = max;
}

nex::AABB::AABB(glm::vec3&& min, glm::vec3&& max)
{
	this->min = std::move(min);
	this->max = std::move(max);
}

nex::AABB::AABB(const AABB2& box)
{
	*this = box;
}

nex::AABB& nex::AABB::operator=(const AABB2& box)
{
	min = resolveInfinity(box.center - box.halfWidth, std::numeric_limits<float>::max());
	max = resolveInfinity(box.center + box.halfWidth, -std::numeric_limits<float>::max());

	return *this;
}

nex::AABB nex::maxAABB(const AABB& a, const AABB& b)
{
	AABB result = a;
	result.min = minVec(result.min, b.min);
	result.min = minVec(result.min, b.max);
	result.max = maxVec(result.max, b.min);
	result.max = maxVec(result.max, b.max);

	return result;
}

nex::AABB nex::operator*(const glm::mat4& trafo, const AABB& box)
{
	//we have to transform all 8 corners and than define the min/max from it.

	std::array<glm::vec3, 8> vecs
	{
		trafo* glm::vec4(box.min, 1.0f),
		trafo* glm::vec4(box.min.x, box.min.y, box.max.z, 1.0f),
		trafo * glm::vec4(box.min.x, box.max.y, box.min.z, 1.0f),
		trafo * glm::vec4(box.max.x, box.min.y, box.min.z, 1.0f),
		trafo * glm::vec4(box.min.x, box.max.y, box.max.z, 1.0f),
		trafo * glm::vec4(box.max.x, box.min.y, box.max.z, 1.0f),
		trafo * glm::vec4(box.max.x, box.max.y, box.min.z, 1.0f),
		trafo * glm::vec4(box.max, 1.0f)
	};

	return nex::calcBounds(vecs.begin(), vecs.end());
}

nex::AABB2::AABB2() : center(glm::vec3(0.0f)), halfWidth(glm::vec3(-std::numeric_limits<float>::max()))
{
}

nex::AABB2::AABB2(const glm::vec3& center, const glm::vec3& halfWidth)
{
	this->center = center;
	this->halfWidth = halfWidth;
}

nex::AABB2::AABB2(glm::vec3&& center, glm::vec3&& halfWidth)
{
	this->center = std::move(center);
	this->halfWidth = std::move(halfWidth);
}

nex::AABB2::AABB2(const AABB& box)
{
	*this = box;
}

nex::AABB2& nex::AABB2::operator=(const AABB& box)
{
	center = resolveInfinity((box.max + box.min) / 2.0f, 0.0f);
	halfWidth = resolveInfinity((box.max - box.min) / 2.0f, -std::numeric_limits<float>::max());

	return *this;
}