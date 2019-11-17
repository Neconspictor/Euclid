#pragma once
#include <glm/mat4x4.hpp>
#include <nex/math/BoundingBox.hpp>

namespace nex
{
	class MeshBatch;

	struct RenderCommand
	{
		MeshBatch* batch;
		const glm::mat4* worldTrafo;
		const glm::mat4* prevWorldTrafo;

		// bounding box information (world space); needed for sorting 
		// meshes by distance and for transparent materials
		const AABB* boundingBox;

		// for skinned meshes; has to point to a valid vector IF not null
		const std::vector<glm::mat4>* mBones;
	};
}