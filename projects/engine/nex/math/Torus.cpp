#include "Torus.hpp"
#include "PolynomialSolver.hpp"

nex::Torus::Torus(const glm::vec3& origin, float radius) : origin(origin), radius(radius)
{
}

nex::Torus::Torus()
{
}

bool nex::Torus::liesOnHull(const glm::vec3& p) const
{
	constexpr auto eps = 0.000001f;
	const auto diff = p - origin;
	return dot(diff, diff) - radius*radius < eps;

	const auto result = PolynomialSolver::solveQuartic(1.0f, 0.0, 2.0, 0, 0);
}
