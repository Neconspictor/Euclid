#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/gui/MenuWindow.hpp>
#include <nex/shadow/ShadowMap.hpp>
#include <interface/GI/voxel_interface.h>
#include <interface/buffers.h>

namespace nex
{
	class Scene;
	class RenderCommandQueue;
	class Renderer;
	struct DirLight;
	class RenderTarget;
	class Texture3D;

	class VoxelConeTracer
	{
	public:

		VoxelConeTracer(bool deferredVoxelizationLighting);
		~VoxelConeTracer();
		
		/**
		 * Specifies whether the voxelization pass should apply lighting or whether it should be deferred 
		 * while voxel texture is updated.
		 * NOTE: This is a compute heavy function (shaders will be recompiled).
		 */
		void deferVoxelizationLighting(bool deferLighting);

		bool getVisualize() const;

		const Texture3D* getVoxelTexture() const;
		Texture3D* getVoxelTexture();

		void setVisualize(bool visualize, int mipMapLevel = 0);

		bool isVoxelLightingDeferred() const;

		void renderVoxels(const glm::mat4& projection, const glm::mat4& view);

		/**
		 * Creates a voxelization representation from the provided render commands (param collection).
		 * If deferred lighting is active, no light contribution is calculated.
		 * @param light : The direct light to use for light contribution. If deferred lighting is active, this argument can be nullptr. Otherwise not!
		 * @param shadows : Used for light contribution. If deferred lighting is active, this argument can be nullptr. Otherwise not!
		 */
		void voxelize(const nex::RenderCommandQueue::ConstBufferCollection& collection,
			const AABB& sceneBoundingBox, const DirLight* light, const ShadowMap* shadows,
			RenderContext& context);

		/**
		 * Updates the voxel texture with previously generated voxel data.
		 * Note: voxelize function has to be called before calling this function.
		 * If deferred voxel lighting is active, the voxelize function only has to be called prior if geometry has changed.
		 * @param light : The direct light to use for light contribution. If deferred lighting isn't active, this argument can be nullptr. Otherwise not!
		 * @param shadows : Used for light contribution. If deferred lighting isn't active, this argument can be nullptr. Otherwise not!
		 */
		void updateVoxelTexture(const DirLight* light, const ShadowMap* shadows);

		void activate(bool isActive);
		bool isActive() const;

	private:

		static const unsigned VOXEL_BASE_SIZE;

		class VoxelizePass;
		class VoxelVisualizePass;
		class VoxelFillComputeLightPass;
		class MipMapTexture3DPass;

		ShaderStorageBuffer mVoxelBuffer;
		std::unique_ptr<Texture3D> mVoxelTexture;

		std::unique_ptr<VoxelizePass> mVoxelizePass;
		std::unique_ptr<VoxelVisualizePass> mVoxelVisualizePass;
		std::unique_ptr<VoxelFillComputeLightPass> mVoxelFillComputeLightPass;
		std::unique_ptr<MipMapTexture3DPass> mMipMapTexture3DPass;

		bool mVisualize;
		int mVoxelVisualizeMipMap;
		bool mUseConeTracing = true;
		std::unique_ptr<RenderTarget> mVoxelizationRT;
		bool mDeferLighting;
	};


	namespace gui {

		class VoxelConeTracerView : public nex::gui::MenuWindow {
		public:
			VoxelConeTracerView(std::string title,
				MainMenuBar* menuBar,
				Menu* menu, 
				VoxelConeTracer* voxelConeTracer,
				const DirLight* light,
				ShadowMap* shadow,
				const Scene* scene,
				RenderContext* context);

			void drawSelf() override;

		private:
			VoxelConeTracer* mVoxelConeTracer;
			const DirLight* mLight;
			ShadowMap* mShadow;
			const Scene* mScene;
			nex::ShadowMap_ConfigurationView mShadowConfig;
			int mMipMap = 0;
			RenderContext* mContext;
		};
	}
}