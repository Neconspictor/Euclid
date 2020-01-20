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

		ProbeManager(unsigned prefilteredSize, unsigned depth);

		ProbeVob* addUninitProbeUnsafe(Probe::Type type, const glm::vec3& position, unsigned storeID = nex::ProbeFactory::INVALID_STOREID);

		Probe* getActiveProbe();

		ProbeFactory* getFactory();
		CubeMapArray* getIrradianceMaps();
		unsigned getNextStoreID() const;
		CubeMapArray* getReflectionMaps();
		const std::vector<std::unique_ptr<Probe>>& getProbes() const;

		ProbeCluster* getProbeCluster();
		const ProbeCluster* getProbeCluster() const;

		/**
		 * Provides for each pbr probe an EnvironmentLight struct.
		 */
		ShaderStorageBuffer* getEnvironmentLightShaderBuffer();

		void setActiveProbe(Probe* probe);

		void update(const nex::Scene::ProbeRange& activeProbes);

		ProbeVob* createUninitializedProbeVob(Probe::Type type, const glm::vec3& position, unsigned storeID);

	private:

		void advanceNextStoreID(unsigned id);

		std::vector<glm::vec4> mProbeSpatials;
		std::vector<std::unique_ptr<Probe>> mProbes;
		std::vector<std::unique_ptr<ProbeVob>> mProbeVobs;
		ShaderStorageBuffer mEnvironmentLights;
		ProbeFactory mFactory;
		Probe* mActive = nullptr;

		unsigned mNextStoreID = 0;

		std::unique_ptr<ProbeCluster> mProbeCluster;
	};
}