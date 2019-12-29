#pragma once
#include <glm/mat4x4.hpp>
#include <nex/math/BoundingBox.hpp>
#include <nex/common/Constants.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/renderer/Drawer.hpp>

namespace nex
{
	class MeshBatch;
	class ShaderStorageBuffer;
	struct RenderCommand;

	using RenderFunction = void(const RenderCommand & command,
		Shader** lastShaderPtr,
		const Constants & constants,
		const ShaderOverride<nex::Shader> & overrides,
		const RenderState * overwriteState);

	/**
	 * Structure for specifying a render command.
	 */
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
		 * If != 0, the render backend uses instanced rendering. 
		 */
		size_t instanceCount = 0;

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
		 * The render function used for rendering the command. 
		 */
		RenderFunction* renderFunc = Drawer::drawCommand;

		/**
		 * Data for the render function.
		 */
		void* data = nullptr;

		/**
		 * Specifies that this command is not deferrable but should be rendered before
		 * transparent commands are rendered.
		 */
		bool renderBeforeTransparent = false;
	};
}