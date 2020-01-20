#pragma once

#include <memory>
#include <nex/GI/Probe.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>

namespace nex
{
	class Scene;
	class Vob;
	class RenderCommandQueue;
	class Renderer;
	class ProbeVob;
	struct DirLight;
	class ProbeCluster;

	class ProbeManager
	{
	public:

		ProbeManager(unsigned reflectionMapSize, unsigned irradianceArraySize, unsigned reflectionArraySize);

		ProbeVob* addUninitProbeUnsafe(Probe::Type type, 
			const glm::vec3& position, 
			std::optional<Texture*> source, 
			unsigned storeID = nex::ProbeFactory::INVALID_STOREID);

		Probe* getActiveIrradianceProbe();
		Probe* getActiveReflectionProbe();

		ProbeFactory* getFactory();
		unsigned getNextStoreID() const;
		const std::vector<std::unique_ptr<Probe>>& getProbes() const;

		ProbeCluster* getProbeCluster();
		const ProbeCluster* getProbeCluster() const;

		/**
		 * Provides for each pbr probe an EnvironmentLight struct.
		 */
		ShaderStorageBuffer* getEnvironmentLightShaderBuffer();

		void setActiveIrradianceProbe(Probe* probe);
		void setActiveReflectionProbe(Probe* probe);

		void update(const nex::Scene::ProbeRange& activeProbes);

		ProbeVob* createUninitializedProbeVob(Probe::Type type, 
			const glm::vec3& position, 
			std::optional<Texture*> source,
			unsigned storeID);

	private:

		void advanceNextStoreID(unsigned id);

		std::vector<glm::vec4> mProbeSpatials;
		std::vector<std::unique_ptr<Probe>> mProbes;
		std::vector<std::unique_ptr<ProbeVob>> mProbeVobs;
		ShaderStorageBuffer mEnvironmentLights;
		ProbeFactory mFactory;
		Probe* mActiveIrradianceProbe = nullptr;
		Probe* mActiveReflectionProbe = nullptr;

		unsigned mNextStoreID = 0;

		std::unique_ptr<ProbeCluster> mProbeCluster;
	};
}