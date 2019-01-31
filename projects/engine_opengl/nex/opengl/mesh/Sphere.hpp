#pragma once
#include <nex/opengl/mesh/MeshGL.hpp>

namespace nex
{
	class Sphere : public MeshGL
	{
	public:
		Sphere(unsigned int xSegments, unsigned int ySegments, Material* material = nullptr);

		~Sphere() override = default;
	};
}
