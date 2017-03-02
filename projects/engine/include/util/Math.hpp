#pragma once
#include <util/Projectional.hpp>

struct AABB
{
	glm::vec3 min;
	glm::vec3 max;
};

FrustumCuboid& operator*=(const FrustumCuboid& frustum, const glm::mat4& matrix);
glm::vec3& perspectiveDivide(const glm::vec3& source, float w);

inline AABB& fromCuboid(const FrustumCuboid& frustum)
{
	AABB bb;
	std::vector<glm::vec3> values;
	values.push_back(frustum.m_near.leftTop);
	values.push_back(frustum.m_near.leftBottom);
	values.push_back(frustum.m_near.rightTop);
	values.push_back(frustum.m_near.rightBottom);
	values.push_back(frustum.m_far.leftTop);
	values.push_back(frustum.m_far.leftBottom);
	values.push_back(frustum.m_far.rightTop);
	values.push_back(frustum.m_far.rightBottom);


	static auto getFromCompare = [](const std::vector< glm::vec3>& vecs, int index, float* currentMin, 
		bool(*compare)(float, float) ) {
		*currentMin = vecs.front()[index];
		for (auto& vec : vecs)
			*currentMin = compare(vec[index], *currentMin) ? vec[index] : *currentMin;
	};

	float x, y, z;
	static auto minCompare = [](float a, float b)->bool { return a < b; };
	static auto maxCompare = [](float a, float b)->bool { return a > b; };
	
	getFromCompare(values, 0, &x, minCompare);
	getFromCompare(values, 1, &y, minCompare);
	getFromCompare(values, 2, &z, maxCompare);
	bb.min = {x,y,z};
	getFromCompare(values, 0, &x, maxCompare);
	getFromCompare(values, 1, &y, maxCompare);
	getFromCompare(values, 2, &z, minCompare);
	bb.max = { x,y,z };
	
	return bb;
}

inline glm::vec3& NDCToCameraSpace(const glm::vec3& source, const glm::mat4& inverseProjection)
{
	glm::vec4 unprojected = inverseProjection * glm::vec4(source, 1);
	return perspectiveDivide(glm::vec3(unprojected), unprojected.w);
}

inline FrustumCuboid& operator*(const glm::mat4& matrix, const FrustumCuboid& frustum) {
	FrustumCuboid result;
	FrustumPlane& nearDest = result.m_near;
	FrustumPlane& farDest = result.m_far;
	const FrustumPlane& nearSource = frustum.m_near;
	const FrustumPlane& farSource = frustum.m_far;

	nearDest.leftBottom = matrix * glm::vec4(nearSource.leftBottom, 1);
	nearDest.leftTop = matrix * glm::vec4(nearSource.leftTop, 1);
	nearDest.rightBottom = matrix * glm::vec4(nearSource.rightBottom, 1);
	nearDest.rightTop = matrix * glm::vec4(nearSource.rightTop, 1);

	farDest.leftBottom = matrix * glm::vec4(farSource.leftBottom, 1);
	farDest.leftTop = matrix * glm::vec4(farSource.leftTop, 1);
	farDest.rightBottom = matrix * glm::vec4(farSource.rightBottom, 1);
	farDest.rightTop = matrix * glm::vec4(farSource.rightTop, 1);

	return result;
}

inline FrustumCuboid& operator*=(const FrustumCuboid& frustum, const glm::mat4& matrix) {
	return matrix * frustum;
}

inline glm::vec3& perspectiveDivide(const glm::vec3& source, float w)
{
	glm::vec3 result = source / w;
	return result;
}