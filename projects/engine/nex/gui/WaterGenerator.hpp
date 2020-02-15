#pragma once

#include <nex/gui/Drawable.hpp>
#include "nex/gui/TextureView.hpp"
#include <nex/scene/Scene.hpp>
#include <nex/math/BoundingBox.hpp>
#include <nex/gui/TextureView.hpp>
#include <nex/gui/TextureViewer.hpp>


namespace nex
{
	class Scene;
	class CascadedShadow;
	class PSSR;
}

namespace nex::gui
{
	class WaterGenerator : public nex::gui::Drawable
	{
	public:
		WaterGenerator(nex::Scene* scene, 
			CascadedShadow* csm,
			PSSR* pssr);
		virtual ~WaterGenerator();

		void setScene(nex::Scene* scene);

	protected:

		void drawSelf() override;

		nex::Scene* mScene;
		nex::CascadedShadow* mCsm;
		nex::PSSR* mPssr;
	};
}