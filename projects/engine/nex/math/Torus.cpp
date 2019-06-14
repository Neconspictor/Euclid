#include <nex/math/Sphere.hpp>

nex::Sphere::Sphere(const glm::vec3& origin, float radius) : origin(origin), radius(radius)
{
}

nex::Sphere::Sphere()
{
}

bool nex::Sphere::liesOnHull(const glm::vec3& p) const
{
	constexpr auto eps = 0.000001f;
	const auto diff = p - origin;
	return dot(diff, diff) - radius*radius < eps;
}
