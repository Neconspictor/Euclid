#include "Torus.hpp"
#include "PolynomialSolver.hpp"

nex::Torus::Torus(float innerRadius, float outerRadius) : innerRadius(innerRadius), outerRadius(outerRadius)
{
}

nex::Torus::Torus()
{
}

bool nex::Torus::isInHull(const glm::vec3& p) const
{
	//constexpr glm::vec3 origin(0.0f);
	constexpr auto eps = 0.000001f;

	const auto distanceToOrigin = length(p);

	if (distanceToOrigin > innerRadius + outerRadius)
		return false;

	if (distanceToOrigin < innerRadius - outerRadius)
		return false;

	// get nearest origin for the outer radius
	const auto originOuter = innerRadius * normalize(glm::vec3(p.x, 0, p.z));
	
	// now check if the point is on the circle defined by originOuter
	// Note: p and originOuter lie on the same x-z plane, 
	// so it is correct to use the distance between the two points!
	const auto distance = length(p - originOuter);
	return distance <= outerRadius;

	/*const auto rIn2 = innerRadius * innerRadius;
	const auto rOut2 = outerRadius * outerRadius;
	const auto x2 = p.x* p.x;
	const auto y2 = p.y * p.y;
	const auto z2 = p.z * p.z;

	auto leftEquation = x2 + y2 + z2 + rIn2 - rOut2;
	leftEquation *= leftEquation;
	auto rightEquation = 4 * rIn2 * (x2 + y2);

	return abs(leftEquation - rightEquation) < eps;*/

	//const auto result = PolynomialSolver::solveQuartic(1.0f, 0.0, 2.0, 0, 0);
}
