#pragma once

#include <memory>
#include <nex/pbr/PbrProbe.hpp>


namespace nex
{
	class PbrProbe;
	class SphereMesh;

	class GlobalIllumination
	{
	public:

		GlobalIllumination(const std::string& compiledProbeDirectory);
		~GlobalIllumination();

		PbrProbe* getProbe();

	private:
		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		std::unique_ptr<SphereMesh> mSphere;
		PbrProbeFactory* mFactory;
	};
}
