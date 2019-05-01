#pragma once

#include <nex/mesh/StaticMesh.hpp>

namespace nex
{
	class HeightMap
	{
	public:
		/**
		 * Vertex data of the height map
		 */
		using Vertex = Mesh::Vertex;

		HeightMap(unsigned xSegments, 
			unsigned zSegments, 
			float worldDimensionX, 
			float worldDimensionZ,
			float worldDimensionMaxHeight);

	private:
		unsigned mXSegments;
		unsigned mZSegments;
		float mWorldDimensionX;
		float mWorldDimensionZ;
		float mWorldDimensionMaxHeight;
		StaticMeshContainer mMeshes;
	};
}
