#pragma once
#include <nex/mesh/SubMesh.hpp>

namespace nex
{
	class Material;

	class Sphere : public Mesh
	{
	public:
		Sphere(unsigned int xSegments, unsigned int ySegments, Material* material = nullptr);

		~Sphere() override = default;
	};
}
