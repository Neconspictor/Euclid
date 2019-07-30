#pragma once
#include <nex/mesh/Mesh.hpp>

namespace nex
{
	class Material;
	struct Frustum;

	class SphereMesh : public Mesh
	{
	public:
		SphereMesh(unsigned int xSegments, unsigned int ySegments, bool finalize = true);

		virtual ~SphereMesh() = default;

		void finalize() override;

	private:

		unsigned mXSegments;
		unsigned mYSegments;

		std::vector<VertexPositionNormalTex> mVertices;
		std::vector<unsigned> mIndices;
	};

	class FrustumMesh : public Mesh
	{
	public:

		FrustumMesh(const Frustum& frustum);

		void finalize() override;

	private:
		std::vector<VertexPositionNormalTex> mVertices;
		std::vector<unsigned> mIndices;
	};
}