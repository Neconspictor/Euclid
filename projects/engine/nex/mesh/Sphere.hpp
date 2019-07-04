#pragma once
#include <nex/mesh/Mesh.hpp>

namespace nex
{
	class Material;

	class SphereMesh : public Mesh
	{
	public:
		SphereMesh(unsigned int xSegments, unsigned int ySegments);

		~SphereMesh() override = default;

	private:
		void init(unsigned int xSegments, unsigned int ySegments);
	};
}