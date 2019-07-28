#pragma once
#include <nex/mesh/Mesh.hpp>

namespace nex
{
	class Material;

	class SphereMesh : public Mesh
	{
	public:
		SphereMesh(unsigned int xSegments, unsigned int ySegments, bool finalize = true);

		~SphereMesh() override = default;

		void finalize() override;

	private:
		unsigned mXSegments;
		unsigned mYSegments;
	};
}