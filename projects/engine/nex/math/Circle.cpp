#include <nex/math/Circle.hpp>

nex::Circle3D::Circle3D(Plane plane, glm::vec3 origin, Real radius) : plane(std::move(plane)), origin(std::move(origin)), radius(radius)
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

nex::Real nex::Circle3D::getRadius() const
{
	return radius;
}

bool nex::Circle3D::isOnCircle(const glm::vec3& point, Real toleranceRange) const
{
	if (!plane.onPlane(point)) return false;

	const auto d = point - origin;
	const auto compare = dot(d, d) - radius * radius;
	return compare <= toleranceRange;
}

nex::MinMaxCircle3D::MinMaxCircle3D(Plane plane, glm::vec3 origin, Real minRadius, Real maxRadius) :
mPlane(plane), mOrigin(origin), mMaxRadius(maxRadius), mMinRadius(minRadius)
{
	assert(mPlane.onPlane(mOrigin));
}

const glm::vec3& nex::MinMaxCircle3D::getOrigin() const
{
	return mOrigin;
}

const nex::Plane& nex::MinMaxCircle3D::getPlane() const
{
	return mPlane;
}

nex::Real nex::MinMaxCircle3D::getMaxRadius() const
{
	return mMaxRadius;
}

nex::Real nex::MinMaxCircle3D::getMinRadius() const
{
	return mMinRadius;
}

bool nex::MinMaxCircle3D::onSurface(const glm::vec3& point) const
{
	if (!mPlane.onPlane(point)) return false;

	const auto d = point - mOrigin;
	const auto distance = dot(d, d);

	return distance >= mMinRadius && distance <= mMaxRadius;
}