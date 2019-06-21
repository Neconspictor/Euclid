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

	private:

		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		PbrProbeFactory* mFactory;
	};
}
