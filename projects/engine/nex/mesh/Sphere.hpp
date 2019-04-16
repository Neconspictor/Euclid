#pragma once
#include <nex/mesh/SubMesh.hpp>

namespace nex
{
	class Material;

	class Sphere : public Mesh
	{
	public:
		Sphere(unsigned int xSegments, unsigned int ySegments);

		~Sphere() override = default;
	};
}
