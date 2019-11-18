#pragma once
#include <glm/mat4x4.hpp>
#include <nex/math/BoundingBox.hpp>

namespace nex
{
	class MeshBatch;
	class ShaderStorageBuffer;

	struct RenderCommand
	{
		MeshBatch* batch;
		const glm::mat4* worldTrafo;
		const glm::mat4* prevWorldTrafo;

		// bounding box information (world space); needed for sorting 
		// meshes by distance and for transparent materials
		const AABB* boundingBox;

		
		// indicates that the shader of the batch needs a bone trafo upload
		bool isBoneAnimated;
		// has to point to a valid vector IF isBoneAnimated is set to true
		const std::vector<glm::mat4>* bones;
		// has to point to a valid buffer IF isBoneAnimated is set to true 
		ShaderStorageBuffer* boneBuffer;
	};
}