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