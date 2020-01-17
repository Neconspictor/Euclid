#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ostream>

namespace glm
{
	std::ostream& operator<<(std::ostream& os, const glm::vec2& vec);
	std::ostream& operator<<(std::ostream& os, const glm::vec3& vec);
	std::ostream& operator<<(std::ostream& os, const glm::vec4& vec);
}

namespace nex
{
	struct Dimension
	{
		unsigned xPos;
		unsigned yPos;
		unsigned width;
		unsigned height;
	};

	/**
	 * Calculates an interpolation factor from an input value and two values.
	 * The result is first clamped and than normalized into range [0, 1].
	 */
	float calcNormalizedInterpolationFactor(float input, float valueA, float valueB);

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


	inline float calcNormalizedInterpolationFactor(float input, float valueA, float valueB)
	{
		const auto diff = valueB - valueA;

		// Note: if diff is zero, result will be +/- INF but than clamped to 0/1
		// So result will always be correct.
		return std::clamp<float>((input - valueA) / diff, 0.0f, 1.0f);
	}

	inline glm::vec3 perspectiveDivide(const glm::vec3& source, float w)
	{
		return source / w;
	}

	glm::vec3 resolveInfinity(const glm::vec3& vec, float defaultValue);

	/**
	 * Provides a rotation from one orientation (src) to another (dest)
	 */
	glm::quat rotate(const glm::vec3& src, const glm::vec3& dest);


	template<class T>
	T square(const T& t) {
		return t * t;
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

	constexpr bool isSmallerEqual(const glm::vec3& a, const glm::vec3& b) noexcept
	{
		return a.x <= b.x 
			&& a.y <= b.y
			&& a.z <= b.z;
	}

	inline bool isValid(float value) noexcept
	{
		return !std::isnan(value) && !std::isinf(value);
	}

	inline bool isValid(double value) noexcept
	{
		return !std::isnan(value) && !std::isinf(value);
	}

	inline bool isValid(const glm::vec3& vec) noexcept
	{
		return isValid(vec.x) && isValid(vec.y) && isValid(vec.z);
	}

	inline bool isValid(const glm::quat& q) noexcept
	{
		return isValid(q.x) && isValid(q.y) && isValid(q.z) && isValid(q.w);
	}

	template<typename T>
	constexpr T sign(const T& t) noexcept {
		return t < 0 ? static_cast<T>(-1.0) : static_cast<T>(1.0);
	}
}