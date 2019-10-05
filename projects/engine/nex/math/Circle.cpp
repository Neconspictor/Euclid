#include <nex/math/Circle.hpp>
#include "Sphere.hpp"
#include "Ray.hpp"
#include "Math.hpp"
#include <nex/util/ExceptionHandling.hpp>

nex::Circle3D::Circle3D(glm::vec3 origin, glm::vec3 normal, float radius) : plane({ normal, origin}),
origin(std::move(origin)), radius(radius)
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

nex::Circle3D::RayIntersection nex::Circle3D::intersects(const Ray& ray, float toleranceRange) const
{
	RayIntersection result;

	//At first do a sphere intersection test for rougher test cases
	const auto sphereTest = Sphere(getOrigin(), getRadius()).intersects(ray);

	if (sphereTest.intersectionCount == 0)
	{
		// No intersections
		result.intersectionCount = 0;
		return result;
	}

	if (sphereTest.intersectionCount == 1)
	{
		// test if the intersection point is located on the circle's plane.
		if (!isOnCircle(ray.getPoint(sphereTest.firstMultiplier), toleranceRange))
		{
			// No intersections
			result.intersectionCount = 0;
			return result;
		}

		// One intersection
		result.intersectionCount = 1;
		result.firstMultiplier = sphereTest.firstMultiplier;
		return result;
	}

	// test both points to be located on the circle.
	const auto firstOnPlane = isOnCircle(ray.getPoint(sphereTest.firstMultiplier), toleranceRange);
	const auto secondOnPlane = isOnCircle(ray.getPoint(sphereTest.secondMultiplier), toleranceRange);

	if (!firstOnPlane && !secondOnPlane)
	{
		// No intersections
		result.intersectionCount = 0;
		return result;
	}

	if (firstOnPlane && !secondOnPlane)
	{
		// One intersection
		result.intersectionCount = 1;
		result.firstMultiplier = sphereTest.firstMultiplier;
		return result;
	}

	if (!firstOnPlane && secondOnPlane)
	{
		// One intersection
		result.intersectionCount = 1;
		result.firstMultiplier = sphereTest.secondMultiplier;
		return result;
	}

	// both intersections are located on the circle
	result.intersectionCount = 2;
	result.firstMultiplier = sphereTest.firstMultiplier;
	result.secondMultiplier = sphereTest.secondMultiplier;

	return result;
}

bool nex::Circle3D::isOnCircle(const glm::vec3& point, float toleranceRange) const
{
	if (!plane.onPlane(point)) return false;

	const auto d = point - origin;
	const auto compare = dot(d, d) - radius * radius;
	return compare <= toleranceRange;
}

bool nex::Circle3D::project(const glm::vec3& point, glm::vec3& projectedPoint) const
{
	const auto planeProjection = plane.project(point);
	if (!plane.onPlane(planeProjection)) throw_with_trace(std::runtime_error("nex::Circle3D::project: plane projected point has to be on plane"));
	assert(plane.onPlane(planeProjection));
	const auto diff = planeProjection - origin;
	const auto direction = normalize(diff);

	if (!isValid(direction.x))
	{
		return false;
	}
	projectedPoint = origin + direction * radius;
	return true;
}

nex::MinMaxCircle3D::MinMaxCircle3D(Plane plane, glm::vec3 origin, float minRadius, float maxRadius) :
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

float nex::MinMaxCircle3D::getMaxRadius() const
{
	return mMaxRadius;
}

float nex::MinMaxCircle3D::getMinRadius() const
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