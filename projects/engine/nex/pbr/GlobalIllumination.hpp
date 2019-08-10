#pragma once

#include <memory>
#include <nex/pbr/PbrProbe.hpp>
#include <nex/util/Array.hpp>
#include <glm/glm.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>

namespace nex
{
	class Scene;
	class Vob;
	class RenderCommandQueue;
	class CubeMap;
	class CubeRenderTarget;
	class TransformPass;
	class PbrDeferred;
	class PbrForward;
	class Renderer;
	class PerspectiveCamera;
	class ProbeVob;
	struct DirLight;

	class GlobalIllumination
	{
	public:

		struct ProbeData {
			glm::vec4 arrayIndex;  // only first component is used
			glm::vec4 positionWorld; // last component isn't used
		};

		using ProbesData = PerformanceCBuffer<ProbeVob::ProbeData>;

		GlobalIllumination(const std::string& compiledProbeDirectory, unsigned prefilteredSize, unsigned depth);
		~GlobalIllumination();

		ProbeVob* addUninitProbeUnsafe(const glm::vec3& position, unsigned storeID = nex::PbrProbe::INVALID_STOREID);
		void bakeProbes(const Scene& scene, Renderer* renderer);
		void bakeProbe(ProbeVob*, const Scene& scene, Renderer* renderer);
		
		PbrProbe* getActiveProbe();
		float getAmbientPower() const;


		PbrProbeFactory* getFactory();
		CubeMapArray* getIrradianceMaps();
		unsigned getNextStoreID() const;
		CubeMapArray* getPrefilteredMaps();
		const std::vector<std::unique_ptr<PbrProbe>>& getProbes() const;
		const ProbesData& getProbesData() const;
		ShaderStorageBuffer* getProbesShaderBuffer();

		void setActiveProbe(PbrProbe* probe);
		void setAmbientPower(float ambientPower);

		void update(const nex::Scene::ProbeRange& activeProbes);

	private:

		class ProbeBakePass;

		void advanceNextStoreID(unsigned id);

		void collectBakeCommands(nex::RenderCommandQueue& queue, const Scene& scene, bool doCulling);
		std::shared_ptr<nex::CubeMap> renderToCubeMap(const nex::RenderCommandQueue & queue,
			Renderer* renderer,
			CubeRenderTarget & renderTarget,
			nex::Camera& camera,
			const glm::vec3 & worldPosition,
			const DirLight& light);

		std::vector<glm::vec4> mProbeSpatials;
		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		std::vector<std::unique_ptr<ProbeVob>> mProbeVobs;
		ShaderStorageBuffer mProbesBuffer;
		ProbesData mProbesData;
		PbrProbeFactory mFactory;
		PbrProbe* mActive;
		std::unique_ptr<ProbeBakePass> mProbeBakePass;
		float mAmbientLightPower;
		std::unique_ptr<PbrDeferred> mDeferred;
		std::unique_ptr<PbrForward> mForward;
		unsigned mNextStoreID;
	};
}