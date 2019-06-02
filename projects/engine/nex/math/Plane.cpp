#include <nex/math/Plane.hpp>

nex::Plane nex::operator*(const glm::mat4& trafo, const Plane& plane)
{
	/**
	 * A plane can be transformed by applying the transpose of the inverse transformation matrix
	 * to the 4D-Vector representing the plane.
	 * For math derivation, see
	 * 'Mathematics for 3D Game Programming and Computer Graphics (Third Edition)' by Eric Lengyel, page 101 (Chapter 5.2.3 Transforming Planes)
	 */
	const glm::vec4 planeVec4(plane.normal, plane.signedDistance);
	const auto trasformed = trafo * planeVec4;
	return { glm::vec3(trasformed), trasformed.w };
}

nex::Plane nex::transform(const glm::mat4& trafo, const Plane& plane)
{
	glm::vec4 vec(plane.normal, plane.signedDistance);
	vec = trafo * vec;
	return { normalize(glm::vec3(vec)), vec.w };
}

nex::Plane nex::normalize(const Plane& plane)
{
	Plane result = plane;
	result.normal = normalize(result.normal);
	return result;
}