#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <nex/Scene.hpp>
//#include <nex/pbr/PbrProbe.hpp>

namespace nex
{
	class Scene;
	class PbrProbe;

	class ProbeGenerator
	{
	public:
		ProbeGenerator(nex::Scene* scene);

		void setScene(nex::Scene* scene);
		void show(bool visible);


		std::unique_ptr<nex::PbrProbe> generate() const;

	protected:
		glm::vec3 mPosition;
		nex::Scene* mScene;
		nex::Scene mProbeVisualizationScene;
		nex::SceneNode* mProbeVisualizationRoot;
	};
}