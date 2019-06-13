#include <nex/math/Plane.hpp>

nex::Plane nex::operator*(const glm::mat4& trafo, const Plane& plane)
{
	/**
	 * A plane can be transformed by applying the transpose of the inverse transformation matrix
	 * to the 4D-Vector representing the plane.
	 * For math derivation, see
	 * 'Mathematics for 3D Game Programming and Computer Graphics (Third Edition)' by Eric Lengyel, page 101 (Chapter 5.2.3 Transforming Planes)
	 */
	const glm::vec4 planeVec4(plane.mNormal, plane.mSignedDistance);
	const auto trasformed = trafo * planeVec4;
	return { glm::vec3(trasformed), trasformed.w };
}

nex::Plane nex::transform(const glm::mat4& trafo, const Plane& plane)
{
	glm::vec4 vec(plane.mNormal, plane.mSignedDistance);
	vec = trafo * vec;
	return { normalize(glm::vec3(vec)), vec.w };
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
	mSignedDistance = dot(normal, pointOnPlane);
}

nex::Plane::Plane(float x, float y, float z, float d) : mNormal(x,y,z), mSignedDistance(d)
{
	normalize(*this);
}

bool nex::Plane::onPlane(const glm::vec3& v) const
{
	constexpr auto eps = 0.000001f;
	return abs(dot(mNormal, v) - mSignedDistance) < eps;
}

glm::vec3 nex::Plane::project(const glm::vec3& p) const
{
	// get a point on the plane
	const auto n = normalize(mNormal);
	const glm::vec3 o = mNormal * mSignedDistance;

	auto v = p - o;
	const auto vLen = length(v);
	v = normalize(v);

	const auto vDotN = dot(v, n);

	//Note: if angle between v and normal is < 90°  n is in direction p - p'
	// But we need the direction p'- p, thus we use -vDotN.
	const auto distance = -vDotN * vLen;

	return p + distance * n;
}


nex::Plane nex::normalize(const Plane& plane)
{
	Plane result = plane;
	result.mSignedDistance *= length(result.mNormal);
	result.mNormal = normalize(result.mNormal);
	return result;
}