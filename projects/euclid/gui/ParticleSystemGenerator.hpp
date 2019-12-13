#pragma once

#include <nex/gui/Drawable.hpp>
#include "nex/gui/TextureView.hpp"
#include <nex/scene/Scene.hpp>
#include <nex/math/BoundingBox.hpp>
#include <nex/gui/TextureView.hpp>
#include <nex/gui/TextureViewer.hpp>


namespace nex
{
	class Camera;
	class Scene;
	class VisualizationSphere;
	class ParticleShader;
}

namespace nex::gui
{
	class ParticleSystemGenerator : public nex::gui::Drawable
	{
	public:
		ParticleSystemGenerator(nex::Scene* scene, VisualizationSphere* sphere, nex::Camera* camera, nex::Window* window,
			nex::ParticleShader* shader);
		virtual ~ParticleSystemGenerator();

		void setCamera(nex::Camera* camera);
		void setScene(nex::Scene* scene);

		void setVisible(bool visible) override;

	protected:

		void createParticleSystem(const glm::vec3& position, Texture* texture);

		void drawSelf() override;

		void onCanvasResizeSelf(unsigned width, unsigned height) override;

		nex::gui::TextureViewer mTextureViewer;

		VisualizationSphere* mSphere;
		nex::Camera* mCamera;
		nex::Scene* mScene;
		float mPlacementOffset;

		float mAverageLifeTime;
		float mAverageScale;
		float mAverageSpeed;
		AABB mBoundingBox;
		float mGravityInfluence;
		int mMaxParticles;
		float mPps;
		float mRotation;
		bool mRandomizeRotation;

		glm::uvec2 mCanvasSize;

		nex::ParticleShader* mShader;
	};
}