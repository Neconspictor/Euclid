#pragma once

#include <nex/math/BoundingBox.hpp>
#include <nex/GI/Probe.hpp>

namespace nex
{
	class DiffuseGiProbeCluster {
	public:

		DiffuseGiProbeCluster(const nex::AABB& extensionWS, const glm::uvec3& clusterSize);

		/**
		 * Maps a world space position to a matching cluster index.
		 * Note: No range checking is performed. The resulting index could be invalid!
		 */
		glm::ivec3 mapToCluster(glm::vec3 position) const;

		/**
		 * Checks if an index points to an existing cluster element.
		 */
		bool isInCluster(const glm::ivec3& index) const;

		/**
		 * Clamps an index to a valid cluster cell.
		 */
		glm::ivec3 clampIndex(const glm::ivec3& index);

		void addProbe(const Probe);

	private:
		nex::AABB mExtention;
		glm::vec3 mOrigin;
		glm::ivec3 mClusterSize;
		glm::vec3 mCellDimension;
	};
}