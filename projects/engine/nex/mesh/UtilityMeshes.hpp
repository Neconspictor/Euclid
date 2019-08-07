#pragma once
#include <nex/mesh/Mesh.hpp>

namespace nex
{
	class Material;
	struct Frustum;
	struct AABB;

	class SphereMesh : public Mesh
	{
	public:
		SphereMesh(unsigned int xSegments, unsigned int ySegments, bool finalize = true);

		virtual ~SphereMesh() = default;
	};

	class FrustumMesh : public Mesh
	{
	public:

		FrustumMesh(const Frustum& frustum);
	};

	class MeshAABB : public Mesh {
	public:
		MeshAABB(const AABB& box, nex::Topology topology);

	private:
		void createLineMesh(const AABB& box);
		void createTriangleMesh(const AABB& box);
	};
}