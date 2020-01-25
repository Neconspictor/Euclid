#include <nex/GI/DiffuseGiProbeCluster.hpp>
#include <glm/glm.hpp>
#include <glm/common.hpp>

nex::DiffuseGiProbeCluster::DiffuseGiProbeCluster(const nex::AABB& extension, const glm::uvec3& clusterSize) :
	mExtention(extension), mClusterSize(clusterSize)
{
	mOrigin = mExtention.min;
	mCellDimension = (mExtention.max - mExtention.min) / glm::vec3(clusterSize);
}

glm::ivec3 nex::DiffuseGiProbeCluster::mapToCluster(glm::vec3 position) const
{
	position -= mOrigin;
	position = position / mCellDimension;
	return glm::ivec3(position);
}

bool nex::DiffuseGiProbeCluster::isInCluster(const glm::ivec3& index) const
{
	return glm::all(glm::greaterThanEqual(index, glm::ivec3(0)))
		&& glm::all(glm::lessThan(index, mClusterSize));
}

glm::ivec3 nex::DiffuseGiProbeCluster::clampIndex(const glm::ivec3& index)
{
	return glm::clamp(index, glm::ivec3(0), mClusterSize);
}