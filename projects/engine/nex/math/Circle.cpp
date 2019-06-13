#include <nex/math/Circle.hpp>

nex::Circle3D::Circle3D(Plane plane, glm::vec3 origin, float radius) : plane(std::move(plane)), origin(std::move(origin)), radius(radius)
{
	assert(this->plane.onPlane(this->origin));
}

const glm::vec3& nex::Circle3D::getOrigin() const
{
	return origin;
}

const nex::Plane& nex::Circle3D::getPlane() const
{
	return plane;
}

float nex::Circle3D::getRadius() const
{
	return radius;
}

bool nex::Circle3D::isOnCircle(const glm::vec3& point, float toleranceRange) const
{
	if (!plane.onPlane(point)) return false;

	const auto d = point - origin;
	const auto compare = dot(d, d) - radius * radius;
	return compare <= toleranceRange;
}