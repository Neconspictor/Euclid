#pragma once

#include <nex/mesh/StaticMesh.hpp>

namespace nex
{
	class Texture2D;
	class Sampler;

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


		HeightMap(const HeightMap&) = delete;
		HeightMap(HeightMap&&) = default;
		HeightMap& operator=(const HeightMap&) = delete;
		HeightMap& operator=(HeightMap&&) = default;

		~HeightMap();

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

		Sampler* getHeightSampler();

		Texture2D* getHeightTexture();

		glm::vec3 getWorldDimension() const;

		/**
		 * Provides the vertex count for the (x,z) plane
		 * the result will look like this:
		 *  - x component: vertex count in x direction
		 *  - y component: vertex count in z direction
		 */
		glm::uvec2 getVertexCount() const;

	private:

		struct TBN
		{
			glm::vec3 tangent;
			glm::vec3 bitangent;
			glm::vec3 normal;
		};
		
		
		unsigned mColumns;
		unsigned mRows;
		float mWorldDimensionX;
		float mWorldDimensionZ;
		float mWorldDimensionMaxHeight;
		StaticMeshContainer mMeshes;
		std::unique_ptr<Sampler> mHeightSampler;
		std::unique_ptr<Texture2D> mHeightTexture;
		


		/**
		 * Calculates a (unnormalized) normal from three positions in CCW order
		 */
		static glm::vec3 calcNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

		/**
		 * Calculates a (unnormalized) TBN matrix from three vertices in CCW order. 
		 */
		static TBN calcTBN(const Vertex& a, const Vertex& b, const Vertex& c);

		glm::vec3 calcPosition(float row, float column, const std::vector<float>& heights) const;

		void generateTBN(std::vector<Vertex>& vertices, int row, int column) const;


		int getColumn(int index) const;
		int getIndex(int row, int column) const;
		int getRow(int index) const;

		bool isInRange(int row, int column) const;

		static bool isValidTBN(const TBN& tbn);


		/**
		 * samples a position by an index and a list of heights.
		 * If the index is out of range (negative or greater/equal the size of heights)
		 * Nothing is done.
		 */
		bool sample(const std::vector<float>& heights, int row, int column, glm::vec3& out) const;

		float sampleHeight(const std::vector<float>& heights, int row, int column) const;
	};
}