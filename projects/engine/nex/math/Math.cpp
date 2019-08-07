#include <nex/math/Math.hpp>
#include <algorithm>
#include "Constant.hpp"

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/quaternion.hpp>

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

glm::vec3 nex::resolveInfinity(const glm::vec3& vec, float defaultValue)
{
	glm::vec3 result = vec;
	for (int i = 0; i < 3; ++i) {
		if (!isValid(result[i]))
			result[i] = defaultValue;
	}

	return result;
}

glm::quat nex::rotate(const glm::vec3& s, const glm::vec3& d)
{
	const auto src = normalize(s);
	const auto dest = normalize(d);
	constexpr auto eps2 = 0.00001 * 0.00001;

	glm::vec3 rotationAxis = src;
	const float angle = acos(dot(src, dest));
	const auto diff = (angle - nex::PI);

	if (diff * diff < eps2)
	{
		// get any vector that is never axis nor -axis
		constexpr glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		constexpr glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);

		glm::vec3 A = yAxis;
		if (glm::length2(src - A) < eps2 || glm::length2(src + A) < eps2)
		{
			A = xAxis;
		}

		// rotation axis is a vector that is orthogonal to axis
		rotationAxis = normalize(cross(src, A));
	}
	else if (angle > eps2)
	{
		rotationAxis = normalize(cross(src, dest));
	}

	return glm::rotate(glm::quat(1, 0, 0, 0), angle, rotationAxis);
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

std::ostream& glm::operator<<(std::ostream& os, const glm::vec4& vec)
{
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << "," << vec.w << ")";
	return os;
}