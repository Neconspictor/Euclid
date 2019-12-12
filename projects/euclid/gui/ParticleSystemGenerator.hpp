#pragma once

#include <nex/gui/Drawable.hpp>
#include "nex/gui/TextureView.hpp"
#include <nex/scene/Scene.hpp>


namespace nex
{
	class Camera;
	class Scene;
	class VisualizationSphere;
}

namespace nex::gui
{
	class ParticleSystemGenerator : public nex::gui::Drawable
	{
	public:
		ParticleSystemGenerator(nex::Scene* scene, VisualizationSphere* sphere, nex::Camera* camera);
		virtual ~ParticleSystemGenerator();

		void setCamera(nex::Camera* camera);
		void setScene(nex::Scene* scene);

		void setVisible(bool visible) override;

	protected:

		void drawSelf() override;

		VisualizationSphere* mSphere;
		nex::Camera* mCamera;
		nex::Scene* mScene;
		float mPlacementOffset;
	};
}