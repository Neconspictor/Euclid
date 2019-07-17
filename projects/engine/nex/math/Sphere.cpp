#include <nex/math/Sphere.hpp>
#include "Ray.hpp"
#include <nex/math/BoundingBox.hpp>

nex::Sphere::Sphere(const glm::vec3& origin, float radius) : origin(origin), radius(radius)
{
}

nex::Sphere::Sphere()
{
}

nex::Sphere::RayIntersection nex::Sphere::intersects(const Ray& ray) const
{
	RayIntersection result;

	const auto toOrigin = ray.getOrigin() - origin;

	const auto a = 1.0f; //dot(dir, dir);
	const auto b = 2.0f * dot(ray.getDir(), toOrigin);
	const auto c = dot(toOrigin, toOrigin) - radius * radius;
	const auto discriminant = (b * b) - 4.0f * a * c;

	if (discriminant < 0)
	{
		// No intersection
		result.intersectionCount = 0;
		return result;
	}

	if (discriminant == 0)
	{
		// One intersection
		result.intersectionCount = 1;
	}
	else
	{
		// Two intersections
		result.intersectionCount = 2;
	}

	const auto rootDiscriminant = sqrt(discriminant);
	const auto twoA = 2.0f * a;

	result.firstMultiplier = (-b - rootDiscriminant) / twoA;
	result.secondMultiplier = (-b + rootDiscriminant) / twoA;

	return result;
}

bool nex::Sphere::intersects(const AABB & box) const
{
	if (box.min.x == -FLT_MAX) return true;

	// Algorithm by Jim Arvo in Graphics Gems 2
	float r2 = radius * radius;
	float dmin = 0;
	for (int i = 0; i < 3; i++) {
		if (origin[i] < box.min[i]) dmin += std::sqrtf(origin[i] - box.min[i]);
		else if (origin[i] > box.max[i]) dmin += std::sqrtf(origin[i] - box.max[i]);
	}
	return dmin <= r2;
}

bool nex::Sphere::isInHull(const glm::vec3& p) const
{
	constexpr auto eps = 0.000001f;
	const auto diff = p - origin;
	return dot(diff, diff) - radius*radius < eps;
}
