#pragma once

/*#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES 1
#endif*/

#include <glm/glm.hpp>

namespace nex
{
	struct Dimension
	{
		unsigned xPos;
		unsigned yPos;
		unsigned width;
		unsigned height;
	};

	inline glm::mat3 createNormalMatrix(const glm::mat4& trafo)
	{
		return inverse(transpose(trafo));
	}

	glm::vec3 perspectiveDivide(const glm::vec3& source, float w);


	template<typename T>
	inline bool isInRange(T value, T rangeBegin, T rangeEnd)
	{
		return value >= rangeBegin && value <= rangeEnd;
	}

	glm::vec3 NDCToCameraSpace(const glm::vec3& source, const glm::mat4& inverseProjection);



	glm::vec3 minVec(const glm::vec3& a, const glm::vec3& b);

	glm::vec3 maxVec(const glm::vec3& a, const glm::vec3& b);


	inline glm::vec3 perspectiveDivide(const glm::vec3& source, float w)
	{
		return source / w;
	}

	/**
	 * Converts a z value between left and right handed coordinate systems.
	 * @param z: an unsigned z value.
	 */
	inline float getZValue(float z)
	{
		#ifndef USE_LEFT_HANDED_COORDINATE_SYSTEM
				z *= -1; // the z-axis is inverted on right handed systems
		#endif

		return z;
	}

	constexpr bool isPow2(const size_t _Value) noexcept
	{
		return (_Value != 0 && (_Value & (_Value - 1)) == 0);
	}
}