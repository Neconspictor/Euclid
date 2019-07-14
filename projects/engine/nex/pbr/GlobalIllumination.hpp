#pragma once

#include <memory>
#include <nex/pbr/PbrProbe.hpp>


namespace nex
{
	class PbrProbe;
	class Scene;
	class Vob;

	class GlobalIllumination
	{
	public:

		GlobalIllumination(const std::string& compiledProbeDirectory, unsigned prefilteredSize, unsigned depth);
		~GlobalIllumination();

		const std::vector<std::unique_ptr<PbrProbe>>& getProbes() const;


		Vob* createVobUnsafe(PbrProbe* probe, Scene& scene);
		void addProbe(std::unique_ptr<PbrProbe>);

		PbrProbe* getActiveProbe();

		CubeMapArray* getIrradianceMaps();
		CubeMapArray* getPrefilteredMaps();

		PbrProbeFactory* getFactory();

		void setActiveProbe(PbrProbe* probe);

	private:

		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		PbrProbeFactory mFactory;
		PbrProbe* mActive;
	};
}