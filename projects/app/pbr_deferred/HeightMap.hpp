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

		/**
		 * Creates a new height map.
		 * 
		 * @param xSegments : Specifies the segments (number of edges) in x direction. Has to be minimal 1.
		 *					  The height map will consist of (xSegments+1) vertices in x direction.					  
		 * @param zSegments : Specifies the segments (number of edges) in z direction. Has to be minimal 1.
		 *					  The height map will consist of (zSegments+1) vertices in z direction.
		 * @param worldDimensionX : The dimension of the height map in x direction.
		 * @param worldDimensionZ : The dimension of the height map in z direction.
		 * @param worldDimensionMaxHeight: The maximum y value, a height value can reach. 
		 * @param heights: the height values for each vertex in the normalized range [0, 1].
		 *				   It is expected that the height values are laid out in rows, 
		 *				   from bottom to top, each row going from left to right. In sum, heights has to contain 
		 *				   (xSegments+1)*(zSegments+1) values.
		 *				   
		 * @throws runtime_error: If heights doesn't have  (xSegments+1)*(zSegments+1) values.	   
		 *				   
		 */
		HeightMap(unsigned xSegments, 
			unsigned zSegments, 
			float worldDimensionX, 
			float worldDimensionZ,
			float worldDimensionMaxHeight,
			const std::vector<float>& heights);

		/**
		 * Creates a height map with zero height
		 */
		static HeightMap createZero(unsigned xSegments,
			unsigned zSegments,
			float worldDimensionX,
			float worldDimensionZ);

		/**
		 * Creates a height map with random heights
		 */
		static HeightMap createRandom(unsigned xSegments,
			unsigned zSegments,
			float worldDimensionX,
			float worldDimensionZ,
			float worldDimensionMaxHeight);


		Mesh* getMesh();

	private:
		unsigned mXSegments;
		unsigned mZSegments;
		float mWorldDimensionX;
		float mWorldDimensionZ;
		float mWorldDimensionMaxHeight;
		StaticMeshContainer mMeshes;
	};
}
