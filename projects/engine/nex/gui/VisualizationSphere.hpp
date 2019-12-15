#pragma once
#include <nex/scene/Vob.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/effects/SimpleColorPass.hpp>

namespace nex
{
	class Scene;

	class VisualizationSphere {
	public:

		VisualizationSphere(Scene* scene);
		~VisualizationSphere();

		void show(bool visible);

		nex::Vob* getVob();
		const nex::Vob* getVob() const;

		bool isVisible() const;

	private:
		nex::Scene* mScene;
		nex::Vob mVisualizationVob;
		nex::MeshGroup mVisualizationMG;
		bool mIsVisible;
	};
	
}