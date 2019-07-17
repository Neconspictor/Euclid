#pragma once

#include <memory>
#include <nex/pbr/PbrProbe.hpp>
#include <nex/util/Array.hpp>
#include <glm/glm.hpp>
#include <nex/shader/ShaderBuffer.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>

namespace nex
{
	class PbrProbe;
	class Scene;
	class Vob;
	class RenderCommandQueue;
	class CubeMap;
	class CubeRenderTarget;
	class TransformPass;

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

		void bakeProbes(const Scene& scene);

		const std::vector<std::unique_ptr<PbrProbe>>& getProbes() const;


		ProbeVob* createVobUnsafe(PbrProbe* probe, Scene& scene);
		void addProbe(std::unique_ptr<PbrProbe>);

		PbrProbe* getActiveProbe();
		PbrProbeFactory* getFactory();
		CubeMapArray* getIrradianceMaps();
		CubeMapArray* getPrefilteredMaps();
		const ProbesData& getProbesData() const;
		ShaderStorageBuffer* getProbesShaderBuffer();

		void setActiveProbe(PbrProbe* probe);

		void update(const nex::Scene::ProbeRange& activeProbes);

	private:

		void collectBakeCommands(nex::RenderCommandQueue& queue, const Scene& scene, bool doCulling);
		std::shared_ptr<nex::CubeMap> renderToCubeMap(const nex::RenderCommandQueue::BufferCollection & buffers,
			TransformPass& pass,
			CubeRenderTarget & renderTarget,
			const glm::vec3 & worldPosition,
			const glm::mat4 & projection);

		std::vector<glm::vec4> mProbeSpatials;
		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		ShaderStorageBuffer mProbesBuffer;
		ProbesData mProbesData;
		PbrProbeFactory mFactory;
		PbrProbe* mActive;
	};
}