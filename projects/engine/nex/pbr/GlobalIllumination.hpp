#pragma once

#include <memory>
#include <nex/pbr/PbrProbe.hpp>
#include <nex/util/Array.hpp>
#include <glm/glm.hpp>
#include <nex/shader/ShaderBuffer.hpp>

namespace nex
{
	class PbrProbe;
	class Scene;
	class Vob;

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

		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		ShaderStorageBuffer mProbesBuffer;
		ProbesData mProbesData;
		PbrProbeFactory mFactory;
		PbrProbe* mActive;
	};
}