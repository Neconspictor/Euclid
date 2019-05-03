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
		 * @param rows : Specifies the number of vertices in z direction. Has to be minimal 2.			  
		 * @param columns : Specifies the number of vertices in x direction. Has to be minimal 2.
		 * @param worldDimensionZ : The dimension of the height map in z direction.
		 * @param worldDimensionMaxHeight: The maximum y value, a height value can reach. 
		 * @param worldDimensionX : The dimension of the height map in x direction.
		 * @param heights: the height values for each vertex in the normalized range [0, 1].
		 *				   It is expected that the height values are laid out in rows, 
		 *				   from bottom to top, each row going from left to right. In sum, heights has to contain 
		 *				   (xSegments+1)*(zSegments+1) values.
		 *				   
		 * @throws runtime_error: If heights doesn't have  (xSegments+1)*(zSegments+1) values.	   
		 *				   
		 */
		HeightMap(unsigned rows, 
			unsigned columns, 
			float worldDimensionZ,
			float worldDimensionMaxHeight,
			float worldDimensionX, 
			const std::vector<float>& heights);

		/**
		 * Creates a height map with zero height
		 */
		static HeightMap createZero(unsigned rows,
			unsigned columns,
			float worldDimensionZ,
			float worldDimensionX);

		/**
		 * Creates a height map with random heights
		 */
		static HeightMap createRandom(unsigned rows,
			unsigned columns,
			float worldDimensionZ,
			float worldDimensionMaxHeight,
			float worldDimensionX);


		Mesh* getMesh();

	private:
		unsigned mColumns;
		unsigned mRows;
		float mWorldDimensionX;
		float mWorldDimensionZ;
		float mWorldDimensionMaxHeight;
		StaticMeshContainer mMeshes;

		void addPosition(std::vector<glm::vec3>& vec, const std::vector<float>& heights, int row, int column) const;

		/**
		 * Calculates a (normalized) normal from three positions in CCW order
		 */
		static glm::vec3 calcNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

		glm::vec3 calcPosition(float row, float column, const std::vector<float>& heights) const;

		void generateTBN(Vertex& vertex, const std::vector<float>& heights, int row, int column) const;


		int getColumn(int index) const;
		int getIndex(int row, int column) const;
		int getRow(int index) const;
		/**
		 * samples a position by an index and a list of heights.
		 * If the index is out of range (negative or greater/equal the size of heights)
		 * Nothing is done.
		 */
		bool sample(const std::vector<float>& heights, int row, int column, glm::vec3& out) const;

		float sampleHeight(const std::vector<float>& heights, int row, int column) const;
	};
}