#pragma once

#include <nex/GI/ProbeBaker.hpp>
#include <nex/GI/ProbeManager.hpp>
#include <nex/GI/VoxelConeTracer.hpp>

namespace nex
{
	class GlobalIllumination
	{
	public:
		
		GlobalIllumination(unsigned reflectionMapSize, unsigned irradianceArraySize, unsigned reflectionArraySize, bool deferredVoxelizationLighting);

		float getAmbientPower() const;

		ProbeBaker* getProbeBaker();
		const ProbeBaker* getProbeBaker() const;

		ProbeManager* getProbeManager();
		const ProbeManager* getProbeManager() const;

		VoxelConeTracer* getVoxelConeTracer();
		const VoxelConeTracer* getVoxelConeTracer() const;

		void setAmbientPower(float ambientPower);


	private:

		ProbeBaker mProbeBaker;
		ProbeManager mProbeManager;
		VoxelConeTracer mVoxelConeTracer;

		float mAmbientLightPower;
	};
}