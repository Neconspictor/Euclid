#pragma once

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES 1
#endif

#include <glm/glm.hpp>

namespace nex
{


	namespace util {
		constexpr double PI = 3.14159265358979323846;
	}

	struct AABB
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	struct Dimension
	{
		unsigned xPos;
		unsigned yPos;
		unsigned width;
		unsigned height;
	};

	glm::vec3 perspectiveDivide(const glm::vec3& source, float w);

	template<typename T>
	inline bool isInRange(T value, T rangeBegin, T rangeEnd)
	{
		return value >= rangeBegin && value <= rangeEnd;
	}

	inline glm::vec3 NDCToCameraSpace(const glm::vec3& source, const glm::mat4& inverseProjection)
	{
		glm::vec4 unprojected = inverseProjection * glm::vec4(source, 1);
		return std::move(perspectiveDivide(glm::vec3(unprojected), unprojected.w));
	}


	inline glm::vec3 perspectiveDivide(const glm::vec3& source, float w)
	{
		glm::vec3 result = source / w;
		return std::move(result);
	}

	inline glm::vec3 minVec(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::vec3(std::min<float>(a.x, b.x),
			std::min<float>(a.y, b.y),
			std::min<float>(a.z, b.z));
	}

	inline glm::vec3 maxVec(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::vec3(std::max<float>(a.x, b.x),
			std::max<float>(a.y, b.y),
			std::max<float>(a.z, b.z));
	}
}