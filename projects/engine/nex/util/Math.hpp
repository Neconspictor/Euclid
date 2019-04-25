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

	/**
	 * Represents the plane equation as a 
	 */
	struct Plane
	{
		glm::vec3 normal = {0,0,-1};
		float signedDistance = 0;

		Plane() {}
		Plane(glm::vec3 normal , float distance) : normal(normal), signedDistance(distance) {}
		Plane(float x, float y, float z, float d)
		{
			normal = { x,y,z };
			signedDistance = d;
		}
	};

	glm::mat3 createNormalMatrix(const glm::mat4& trafo);

	Plane normalize(const Plane& plane);

	/**
	 * Transforms a plane by a transformation matrix
	 */
	Plane operator*(const glm::mat4& trafo, const Plane& plane);
	Plane transform(const glm::mat4& trafo, const Plane& plane);

	glm::vec3 perspectiveDivide(const glm::vec3& source, float w);

	inline glm::mat3 createNormalMatrix(const glm::mat4& trafo)
	{
		return transpose(inverse(glm::mat3(trafo)));
	}


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


	inline Plane operator*(const glm::mat4& mat, const Plane& plane)
	{
		/**
		 * A plane can be transformed by applying the transpose of the inverse transformation matrix
		 * to the 4D-Vector representing the plane.
		 * For math derivation, see 
		 * 'Mathematics for 3D Game Programming and Computer Graphics (Third Edition)' by Eric Lengyel, page 101 (Chapter 5.2.3 Transforming Planes)
		 */
		const auto trafo = transpose(inverse(mat));
		glm::vec4 planeVec4(plane.normal, plane.signedDistance);
		auto trasformed = trafo * planeVec4;
		return { glm::vec3(trasformed), trasformed.w };
	}

	inline Plane transform(const glm::mat4& trafo, const Plane& plane)
	{
		glm::vec4 vec(plane.normal, plane.signedDistance);
		vec = trafo * vec;
		return {normalize(glm::vec3(vec)), vec.w};
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

	inline Plane normalize(const Plane& plane)
	{
		Plane result = plane;
		result.normal = normalize(result.normal);
		return result;
	}


	inline glm::vec3 perspectiveDivide(const glm::vec3& source, float w)
	{
		return source / w;
	}
}