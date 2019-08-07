#include <nex/math/Plane.hpp>
#include "Ray.hpp"
#include "Math.hpp"

nex::Plane nex::operator*(const glm::mat4& trafo, const Plane& plane)
{
	/**
	 * A plane can be transformed by applying the transpose of the inverse transformation matrix
	 * to the 4D-Vector representing the plane.
	 * For math derivation, see
	 * 'Mathematics for 3D Game Programming and Computer Graphics (Third Edition)' by Eric Lengyel, page 101 (Chapter 5.2.3 Transforming Planes)
	 */
	return transformWithTransposeInverse(transpose(inverse(trafo)), plane);
}

nex::Plane nex::transformWithTransposeInverse(const glm::mat4& transposeInverseTrafo, const Plane& plane)
{

	const glm::vec4 planeVec4(plane.mNormal, plane.mSignedDistance);
	const auto trasformed = transposeInverseTrafo * planeVec4;
	return { glm::vec3(trasformed), trasformed.w };
}

nex::Plane::Plane()
{
}

nex::Plane::Plane(glm::vec3 normal, float distance) : mNormal(normal), mSignedDistance(distance)
{
	normalize(*this);
}

nex::Plane::Plane(glm::vec3 normal, glm::vec3 pointOnPlane) : mNormal(normalize(normal))
{
	mSignedDistance = -dot(normal, pointOnPlane);
}

nex::Plane::Plane(float x, float y, float z, float d) : mNormal(x,y,z), mSignedDistance(d)
{
	normalize(*this);
}

nex::Plane::RayIntersection nex::Plane::intersects(const Ray& ray) const
{
	const double d = mSignedDistance;
	const glm::dvec3& n = mNormal;
	const glm::dvec3 dirDouble = ray.getDir();
	constexpr auto eps = 0.04f;

	const auto dotDir = dot(n, dirDouble);
	const auto compare = d + dot(n, glm::dvec3(ray.getOrigin()));

	RayIntersection result;

	// if dotDir = 0 we have to calc differently  
	if (abs(dotDir) < eps)
	{
		// No solutions?
		if (abs(compare) < eps)
		{
			result.intersected = false;
			result.parallel = true;
		}
		else // infinite solutions
		{
			// It doesn't matter which point we use (since all points on the ray lie on the plane)
			// just use any valid point
			result.multiplier = 0.0f;
			result.intersected = true;
			result.parallel = true;
		}

	}
	else
	{
		result.multiplier = (long double)compare / (long double)dotDir;
		result.intersected = true;
		result.parallel = false;
	}

	return result;
}

bool nex::Plane::onPlane(const glm::vec3& v) const
{
	constexpr auto eps = 0.00001f;
	return abs(dot(mNormal, v) + mSignedDistance) < eps;
}

glm::vec3 nex::Plane::project(const glm::vec3& p) const
{
	// get a point on the plane
	const auto n = normalize(mNormal);
	const glm::vec3 o = mNormal * (-mSignedDistance);

	auto v = p - o;
	const auto vLen = length(v);
	v = normalize(v);

	const auto vDotN = dot(v, n);

	//Note: if angle between v and normal is < 90°  n is in direction p - p'
	// But we need the direction p'- p, thus we use -vDotN.
	const auto distance = -vDotN * vLen;

	if (!isValid(distance))
		return p;

	return p + distance * n;
}


nex::Plane nex::normalize(const Plane& plane)
{
	Plane result = plane;
	result.mSignedDistance *= length(result.mNormal);
	result.mNormal = normalize(result.mNormal);
	return result;
}