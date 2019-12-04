#pragma once
#include <glm/mat4x4.hpp>
#include <nex/math/BoundingBox.hpp>

namespace nex
{
	class MeshBatch;
	class ShaderStorageBuffer;
	struct RenderCommand;

	using RenderFunction = void(const RenderCommand*);

	struct RenderCommand
	{
		/**
		 * References to mesh material pairs.
		 */
		const MeshBatch* batch = nullptr;

		/**
		 * The world transformation matrix that should be used for all meshes 
		 * rendered with this render command (If instancing is not used)
		 */
		const glm::mat4* worldTrafo = nullptr;

		/**
		 * The previous world transformation matrix.
		 */
		const glm::mat4* prevWorldTrafo = nullptr;

		/** 
		 * Bounding box information (world space); needed for sorting 
		 * meshes by distance and for transparent materials
		 */
		const AABB* boundingBox = nullptr;

		/** 
		 * Indicates that the shader of the batch needs a bone trafo upload
		 */
		bool isBoneAnimated = false;
		
		/** 
		 * Has to point to a valid vector if isBoneAnimated is set to true
		 */
		const std::vector<glm::mat4>* bones = nullptr;
		
		/** 
		 * Has to point to a valid buffer if isBoneAnimated is set to true 
		 */
		ShaderStorageBuffer* boneBuffer = nullptr;

		/**
		 * A custom render function. 
		 * If set to nullptr, the default render functions from the Drawer class are used.
		 */
		RenderFunction* renderFunc = nullptr;

		/**
		 * Data for custom rendering.
		 */
		void* userData = nullptr;
	};
}