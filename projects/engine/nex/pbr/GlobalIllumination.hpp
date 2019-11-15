#pragma once

#include <memory>
#include <nex/pbr/PbrProbe.hpp>
#include <nex/util/Array.hpp>
#include <glm/glm.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/gui/MenuWindow.hpp>
#include <nex/shadow/ShadowMap.hpp>

namespace nex
{
	class Scene;
	class Vob;
	class RenderCommandQueue;
	class CubeMap;
	class CubeRenderTarget;
	class TransformShader;
	class PbrDeferred;
	class PbrForward;
	class Renderer;
	class PerspectiveCamera;
	class ProbeVob;
	struct DirLight;
	class ProbeCluster;
	class MeshContainer;
	class RenderTarget;

	class GlobalIllumination
	{
	public:

		GlobalIllumination(const std::string& compiledProbeDirectory, unsigned prefilteredSize, unsigned depth, bool deferredVoxelizationLighting);
		~GlobalIllumination();

		ProbeVob* addUninitProbeUnsafe(const glm::vec3& position, unsigned storeID = nex::PbrProbe::INVALID_STOREID);
		void bakeProbes(const Scene& scene, Renderer* renderer);
		void bakeProbe(ProbeVob*, const Scene& scene, Renderer* renderer);
		
		/**
		 * Specifies whether the voxelization pass should apply lighting or whether it should be deferred 
		 * while voxel texture is updated.
		 * NOTE: This is a compute heavy function (shaders will be recompiled).
		 */
		void deferVoxelizationLighting(bool deferLighting);

		PbrProbe* getActiveProbe();
		float getAmbientPower() const;


		PbrProbeFactory* getFactory();
		CubeMapArray* getIrradianceMaps();
		unsigned getNextStoreID() const;
		CubeMapArray* getPrefilteredMaps();
		const std::vector<std::unique_ptr<PbrProbe>>& getProbes() const;

		ProbeCluster* getProbeCluster();
		const ProbeCluster* getProbeCluster() const;

		bool getVisualize() const;

		/**
		 * Provides for each pbr probe an EnvironmentLight struct.
		 */
		ShaderStorageBuffer* getEnvironmentLightShaderBuffer();

		UniformBuffer* getVoxelConstants();
		Texture3D* getVoxelTexture();

		bool isConeTracingUsed() const;

		void setActiveProbe(PbrProbe* probe);
		void setAmbientPower(float ambientPower);

		void setUseConetracing(bool use);

		void setVisualize(bool visualize, int mipMapLevel = 0);

		bool isVoxelLightingDeferred() const;

		void update(const nex::Scene::ProbeRange& activeProbes);

		void renderVoxels(const glm::mat4& projection, const glm::mat4& view);

		/**
		 * Creates a voxelization representation from the provided render commands (param collection).
		 * If deferred lighting is active, no light contribution is calculated.
		 * @param light : The direct light to use for light contribution. If deferred lighting is active, this argument can be nullptr. Otherwise not!
		 * @param shadows : Used for light contribution. If deferred lighting is active, this argument can be nullptr. Otherwise not!
		 */
		void voxelize(const nex::RenderCommandQueue::ConstBufferCollection& collection,
			const AABB& sceneBoundingBox, const DirLight* light, const ShadowMap* shadows);

		/**
		 * Updates the voxel texture with previously generated voxel data.
		 * Note: voxelize function has to be called before calling this function.
		 * If deferred voxel lighting is active, the voxelize function only has to be called prior if geometry has changed.
		 * @param light : The direct light to use for light contribution. If deferred lighting isn't active, this argument can be nullptr. Otherwise not!
		 * @param shadows : Used for light contribution. If deferred lighting isn't active, this argument can be nullptr. Otherwise not!
		 */
		void updateVoxelTexture(const DirLight* light, const ShadowMap* shadows);

		void drawTest(const glm::mat4& projection, const glm::mat4& view, Texture* depth);

	private:

		static const unsigned VOXEL_BASE_SIZE;

		class ProbeBakePass;
		class VoxelizePass;
		class VoxelVisualizePass;
		class VoxelFillComputeLightPass;
		class MipMapTexture3DPass;

		void advanceNextStoreID(unsigned id);

		void collectBakeCommands(nex::RenderCommandQueue& queue, const Scene& scene, bool doCulling);
		std::shared_ptr<nex::CubeMap> renderToCubeMap(const nex::RenderCommandQueue & queue,
			Renderer* renderer,
			CubeRenderTarget & renderTarget,
			nex::Camera& camera,
			const glm::vec3 & worldPosition,
			const DirLight& light);

		std::shared_ptr<nex::CubeMap> renderToDepthCubeMap(const nex::RenderCommandQueue& queue,
			Renderer* renderer,
			CubeRenderTarget& renderTarget,
			nex::Camera& camera,
			const glm::vec3& worldPosition,
			const DirLight& light);

		std::vector<glm::vec4> mProbeSpatials;
		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		std::vector<std::unique_ptr<ProbeVob>> mProbeVobs;
		ShaderStorageBuffer mEnvironmentLights;
		ShaderStorageBuffer mVoxelBuffer;
		UniformBuffer mVoxelConstantBuffer;
		std::unique_ptr<Texture3D> mVoxelTexture;
		PbrProbeFactory mFactory;
		PbrProbe* mActive;
		std::unique_ptr<ProbeBakePass> mProbeBakePass;
		std::unique_ptr<VoxelizePass> mVoxelizePass;
		std::unique_ptr<VoxelVisualizePass> mVoxelVisualizePass;
		std::unique_ptr<VoxelFillComputeLightPass> mVoxelFillComputeLightPass;
		std::unique_ptr<MipMapTexture3DPass> mMipMapTexture3DPass;
		float mAmbientLightPower;
		std::unique_ptr<PbrDeferred> mDeferred;
		std::unique_ptr<PbrForward> mForward;
		std::unique_ptr<TransformShader> mIrradianceDepthPass;
		unsigned mNextStoreID;

		std::unique_ptr<ProbeCluster> mProbeCluster;
		std::unique_ptr<MeshContainer> mSphere;
		bool mVisualize;
		int mVoxelVisualizeMipMap;
		bool mUseConeTracing;
		std::unique_ptr<RenderTarget> mVoxelizationRT;
		bool mDeferLighting;
	};


	namespace gui {

		class GlobalIlluminationView : public nex::gui::MenuWindow {
		public:
			GlobalIlluminationView(std::string title,
				MainMenuBar* menuBar,
				Menu* menu, 
				GlobalIllumination* globalIllumination,
				const DirLight* light,
				const ShadowMap* shadow,
				const RenderCommandQueue* queue,
				const Scene* scene);

			void drawSelf() override;

		private:
			GlobalIllumination* mGlobalIllumination;
			const DirLight* mLight;
			const ShadowMap* mShadow;
			const RenderCommandQueue* mQueue;
			const Scene* mScene;
			nex::ShadowMap_ConfigurationView mShadowConfig;
		};
	}
}