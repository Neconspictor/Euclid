#pragma once

#include <memory>
#include <nex/GI/PbrProbe.hpp>
#include <nex/util/Array.hpp>
#include <glm/glm.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/gui/MenuWindow.hpp>

namespace nex
{
	class Scene;
	class Vob;
	class RenderCommandQueue;
	class CubeMap;
	class TransformShader;
	class PbrDeferred;
	class PbrForward;
	class Renderer;
	class ProbeVob;
	struct DirLight;
	class ProbeCluster;
	class MeshGroup;


	class ProbeBaker {
	public:

		ProbeBaker();

		~ProbeBaker();

		void bakeProbes(Scene& scene, const DirLight& light, PbrProbeFactory& factory, Renderer* renderer);
		void bakeProbe(ProbeVob*, const Scene& scene, const DirLight& light, PbrProbeFactory& factory, Renderer* renderer);

	private:

		class ProbeBakePass;

		void collectBakeCommands(nex::RenderCommandQueue& queue, const Scene& scene, bool doCulling);

		std::shared_ptr<nex::CubeMap> renderToCubeMap(const nex::RenderCommandQueue& queue,
			Renderer* renderer,
			CubeRenderTarget& renderTarget,
			nex::Camera& camera,
			const glm::vec3& worldPosition,
			const DirLight& light);

		std::shared_ptr<nex::CubeMap> renderToDepthCubeMap(const nex::RenderCommandQueue& queue,
			Renderer* renderer,
			CubeRenderTarget& renderTarget,
			nex::Camera& camera,
			const glm::vec3& worldPosition,
			const DirLight& light);

		std::unique_ptr<ProbeBakePass> mProbeBakePass;
		std::unique_ptr<PbrDeferred> mDeferred;
		std::unique_ptr<PbrForward> mForward;
		std::unique_ptr<TransformShader> mIrradianceDepthPass;
		std::unique_ptr<MeshGroup> mSphere;
	};

	class ProbeManager
	{
	public:

		ProbeManager(unsigned prefilteredSize, unsigned depth);

		ProbeVob* addUninitProbeUnsafe(const glm::vec3& position, unsigned storeID = nex::PbrProbe::INVALID_STOREID);

		PbrProbe* getActiveProbe();

		PbrProbeFactory* getFactory();
		CubeMapArray* getIrradianceMaps();
		unsigned getNextStoreID() const;
		CubeMapArray* getPrefilteredMaps();
		const std::vector<std::unique_ptr<PbrProbe>>& getProbes() const;

		ProbeCluster* getProbeCluster();
		const ProbeCluster* getProbeCluster() const;

		/**
		 * Provides for each pbr probe an EnvironmentLight struct.
		 */
		ShaderStorageBuffer* getEnvironmentLightShaderBuffer();

		void setActiveProbe(PbrProbe* probe);

		void update(const nex::Scene::ProbeRange& activeProbes);

		ProbeVob* createUninitializedProbeVob(const glm::vec3& position, unsigned storeID);

	private:

		void advanceNextStoreID(unsigned id);

		std::vector<glm::vec4> mProbeSpatials;
		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		std::vector<std::unique_ptr<ProbeVob>> mProbeVobs;
		ShaderStorageBuffer mEnvironmentLights;
		PbrProbeFactory mFactory;
		PbrProbe* mActive = nullptr;

		unsigned mNextStoreID = 0;

		std::unique_ptr<ProbeCluster> mProbeCluster;
	};
}