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
	class Camera;
	struct RenderContext;
}

namespace nex::gui
{
	class OceanGenerator : public nex::gui::Drawable
	{
	public:
		OceanGenerator(nex::Scene* scene, 
			CascadedShadow* csm,
			PSSR* pssr,
			const Camera* camera,
			const RenderContext* renderContext);
		virtual ~OceanGenerator();

		void setScene(nex::Scene* scene);

	protected:

		void drawSelf() override;

		const nex::Camera* mCamera;
		nex::Scene* mScene;
		nex::CascadedShadow* mCsm;
		nex::PSSR* mPssr;
		const nex::RenderContext* mContext;


		unsigned mN = 64;
		unsigned mMaxWaveLength = 64;
		float mDimension = 10.0f;
		float mSpectrumScale = 1.0f;
		glm::vec2 mWindDirection = glm::vec2(0.707f, 0.707f);
		float mWindSpeed = 12.0f;
		float mPeriodTime = 20.0f;
		glm::uvec2 mTileCount = { 1,1 };
		float mMurk = 0.5f;
		float mRoughness = 0.1f;
	};
}