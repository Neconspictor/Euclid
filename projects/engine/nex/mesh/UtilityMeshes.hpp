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
	};

	class FrustumMesh : public Mesh
	{
	public:

		FrustumMesh(const Frustum& frustum);
	};
}