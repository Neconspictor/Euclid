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

		GlobalIllumination(const std::string& compiledProbeDirectory);
		~GlobalIllumination();

		PbrProbe* getProbe();


		Vob* createVob(PbrProbe* probe, Scene& scene);

		void loadHdr();
		void loadProbes(std::unique_ptr<PbrProbe>);

		PbrProbeFactory* getFactory();
		Texture2D* getHdr();

	private:

		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		PbrProbeFactory* mFactory;
		Texture2D* mHdr;
	};
}