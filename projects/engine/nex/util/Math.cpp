#include <nex/util/Math.hpp>

nex::AABB nex::operator*(const glm::mat4& trafo, const AABB& box)
{
	//we have to transform all 8 corners and than define the min/max from it.

	std::array<glm::vec4, 8> vecs;
	vecs[0] = trafo * glm::vec4(box.min.x, box.min.y, box.min.z, 1.0f);
	vecs[1] = trafo * glm::vec4(box.min.x, box.min.y, box.max.z, 1.0f);
	vecs[2] = trafo * glm::vec4(box.min.x, box.max.y, box.min.z, 1.0f);
	vecs[3] = trafo * glm::vec4(box.max.x, box.min.y, box.min.z, 1.0f);
	vecs[4] = trafo * glm::vec4(box.min.x, box.max.y, box.max.z, 1.0f);
	vecs[5] = trafo * glm::vec4(box.max.x, box.min.y, box.max.z, 1.0f);
	vecs[6] = trafo * glm::vec4(box.max.x, box.max.y, box.min.z, 1.0f);
	vecs[7] = trafo * glm::vec4(box.max.x, box.max.y, box.max.z, 1.0f);

	AABB result;
	result.min = vecs[0];
	result.max = vecs[0];

	for (auto i = 0; i < 8; ++i)
	{
		glm::vec3 vec = vecs[i];
		result.min = minVec(result.min, vec);
		result.max = maxVec(result.max, vec);
	}

	return result;

}