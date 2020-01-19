#pragma once

#include <memory>
#include <nex/GI/PbrProbe.hpp>
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

		ProbeVob* addUninitProbeUnsafe(const glm::vec3& position, unsigned storeID = nex::PbrProbe::INVALID_STOREID);

		PbrProbe* getActiveProbe();

		PbrProbeFactory* getFactory();
		CubeMapArray* getIrradianceMaps();
		unsigned getNextStoreID() const;
		CubeMapArray* getPrefilteredMaps();
		const std::vector<std::unique_ptr<PbrProbe>>& getProbes() const;

		ProbeCluster* getProbeCluster();
		const ProbeCluster* getProbeCluster() const;

		/**
		 * Provides for each pbr probe an EnvironmentLight struct.
		 */
		ShaderStorageBuffer* getEnvironmentLightShaderBuffer();

		void setActiveProbe(PbrProbe* probe);

		void update(const nex::Scene::ProbeRange& activeProbes);

		ProbeVob* createUninitializedProbeVob(const glm::vec3& position, unsigned storeID);

	private:

		void advanceNextStoreID(unsigned id);

		std::vector<glm::vec4> mProbeSpatials;
		std::vector<std::unique_ptr<PbrProbe>> mProbes;
		std::vector<std::unique_ptr<ProbeVob>> mProbeVobs;
		ShaderStorageBuffer mEnvironmentLights;
		PbrProbeFactory mFactory;
		PbrProbe* mActive = nullptr;

		unsigned mNextStoreID = 0;

		std::unique_ptr<ProbeCluster> mProbeCluster;
	};
}