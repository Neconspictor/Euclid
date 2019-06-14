#include <nex/math/Math.hpp>

glm::vec3 nex::NDCToCameraSpace(const glm::vec3& source, const glm::mat4& inverseProjection)
{
	glm::vec4 unprojected = inverseProjection * glm::vec4(source, 1);
	return std::move(perspectiveDivide(glm::vec3(unprojected), unprojected.w));
}

glm::vec3 nex::minVec(const glm::vec3& a, const glm::vec3& b)
{
	return glm::vec3(std::min<Real>(a.x, b.x),
	                 std::min<Real>(a.y, b.y),
	                 std::min<Real>(a.z, b.z));
}

glm::vec3 nex::maxVec(const glm::vec3& a, const glm::vec3& b)
{
	return glm::vec3(std::max<Real>(a.x, b.x),
	                 std::max<Real>(a.y, b.y),
	                 std::max<Real>(a.z, b.z));
}

std::ostream& glm::operator<<(std::ostream& os, const glm::vec2& vec)
{
	os << "(" << vec.x << ", " << vec.y << ")";
	return os;
}

std::ostream& glm::operator<<(std::ostream& os, const glm::vec3& vec)
{
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return os;
}