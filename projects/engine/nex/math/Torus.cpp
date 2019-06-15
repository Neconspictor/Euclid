#include "Torus.hpp"
#include <nex/math/Ray.hpp>
#include <nex/math/algebra.hpp>
#include <algorithm>

nex::Torus::Torus(float innerRadius, float outerRadius) : innerRadius(innerRadius), outerRadius(outerRadius)
{
}

nex::Torus::Torus()
{
}

nex::Torus::RayIntersection nex::Torus::intersects(const Ray& ray) const
{
	RayIntersection result;

	/**
	 * For derivation see: http://cosinekitty.com/raytrace/chapter13_torus.html
	 * Note: we use y as the up direction and thus y and z are switched in this implementation.
	 */

	const double A2 = innerRadius * innerRadius;
	const double B2 = outerRadius * outerRadius;
	const auto D = ray.getOrigin();
	const auto E = ray.getDir();

	const double G = 4.0 * A2 * (E.x*E.x + E.z*E.z); // 4A2(E2x+E2y)
	const double H = 8.0 * A2 * (D.x*E.x + D.z*E.z); // 8A2(DxEx+DyEy)
	const double I = 4.0 * A2 * (D.x*D.x + D.z*D.z); //4A2(D2x+D2y)

	//Note: J is 1, since direction vectors of our ray implementation are always normalized
	const double J = 1.0; //dot(E,E); //E2x+E2y+E2z=|E|2
	const double K = 2.0 * dot(D,E); //2(DxEx+DyEy+DzEz)=2(D⋅E)
	const double L = dot(D, D) + A2 - B2; //D2x+D2y+D2z+A2−B2=|D|2+(A2−B2)

	//(J^2)*(u^4) + (2JK)*(u^3)+ (2JL+(K^2)−G)*(u^2) + (2KL−H)u + (L2−I) = 0
	double solutions[4];
	result.intersectionCount = nex::algebra::SolveQuarticEquation(J*J,
		2.0 * J*K,
		2.0 * J*L + K * K - G,
		2.0 * K*L - H,
		L*L - I,
		solutions);

	std::sort(solutions, solutions + result.intersectionCount);

	for (size_t i = 0; i < result.intersectionCount; ++i)
	{
		result.multipliers[i] = static_cast<float>(solutions[i]);
	}



	return result;
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
