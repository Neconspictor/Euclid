#pragma once

#include <glm/glm.hpp>
#include <memory>
//#include <nex/pbr/PbrProbe.hpp>

namespace nex
{
	class PbrProbe;
	class GlobalIllumination;
	class Renderer;
	class VisualizationSphere;
	class Scene;
	class ProbeVob;
	struct DirLight;

	class ProbeGenerator
	{
	public:
		ProbeGenerator(Scene* scene, VisualizationSphere* sphere, nex::GlobalIllumination* globalIllumination, nex::Renderer* renderer);
		virtual ~ProbeGenerator();

		void show(bool visible);


		const glm::vec3& getProbePosition() const;
		float getInfluenceRadius() const;

		nex::ProbeVob* generate(const DirLight& light);

		void update(const glm::vec3& position, float influenceRadius);

	protected:

		Scene* mScene;
		VisualizationSphere* mSphere;
		float mInfluenceRadius;
		nex::GlobalIllumination* mGlobalIllumination;
		nex::Renderer* mRenderer;
	};
}