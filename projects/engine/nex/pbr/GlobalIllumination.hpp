#pragma once

#include <memory>
#include <nex/pbr/PbrProbe.hpp>
#include <nex/util/Array.hpp>

namespace nex
{
	class PbrProbe;
	class Scene;
	class Vob;

	class GlobalIllumination
	{
	public:

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

		void setActiveProbe(PbrProbe* probe);

		void update(const nex::Scene::ProbeRange& activeProbes);

	private:

		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		ProbesData mProbesData;
		PbrProbeFactory mFactory;
		PbrProbe* mActive;
	};
}