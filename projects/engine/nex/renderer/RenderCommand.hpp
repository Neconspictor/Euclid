#pragma once
#include <glm/mat4x4.hpp>
#include <nex/util/Math.hpp>

namespace nex
{
	class Mesh;
	class Material;
	struct RenderCommand
	{
		Mesh* mesh;
		Material* material;
		glm::mat4 worldTrafo;
		glm::mat4 prevWorldTrafo;

		// bounding box information (world space); needed for sorting 
		// meshes by distance and for transparent materials
		AABB boundingBox;
	};
}