#include <nex/math/Math.hpp>

glm::vec3 nex::NDCToCameraSpace(const glm::vec3& source, const glm::mat4& inverseProjection)
{
	glm::vec4 unprojected = inverseProjection * glm::vec4(source, 1);
	return std::move(perspectiveDivide(glm::vec3(unprojected), unprojected.w));
}

glm::vec3 nex::minVec(const glm::vec3& a, const glm::vec3& b)
{
	return glm::vec3(std::min<float>(a.x, b.x),
	                 std::min<float>(a.y, b.y),
	                 std::min<float>(a.z, b.z));
}

glm::vec3 nex::maxVec(const glm::vec3& a, const glm::vec3& b)
{
	return glm::vec3(std::max<float>(a.x, b.x),
	                 std::max<float>(a.y, b.y),
	                 std::max<float>(a.z, b.z));
}