#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <nex/Scene.hpp>
#include <nex/mesh/MeshContainer.hpp>
//#include <nex/pbr/PbrProbe.hpp>

namespace nex
{
	class PbrProbe;
	class SimpleColorPass;
	class Technique;
	class Camera;
	class GlobalIllumination;
	class Renderer;

	class ProbeGenerator
	{
	public:
		ProbeGenerator(nex::Scene* scene, nex::GlobalIllumination* globalIllumination, nex::Renderer* renderer);
		virtual ~ProbeGenerator();

		void setScene(nex::Scene* scene);
		void show(bool visible);


		const glm::vec3& getProbePosition() const;
		float getInfluenceRadius() const;

		nex::ProbeVob* generate();

		void update(const glm::vec3& position, float influenceRadius);

	protected:
		nex::Scene* mScene;
		nex::Vob mProbeVisualizationVob;
		nex::MeshGroup mProbeVisualizationMeshContainer;
		std::unique_ptr<nex::SimpleColorPass> mSimpleColorPass;
		bool mIsVisible;
		float mInfluenceRadius;
		nex::GlobalIllumination* mGlobalIllumination;
		nex::Renderer* mRenderer;
	};
}