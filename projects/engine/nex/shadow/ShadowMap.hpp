#pragma once
#include <nex/texture/Texture.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/texture/RenderTarget.hpp>
#include "nex/gui/Drawable.hpp"
#include <nex/shader/Shader.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/shadow/ShadowCommon.hpp>
#include <nex/gui/TextureView.hpp>

namespace nex
{

	struct DirLight;

	class ShadowMap
	{
	public:

		ShadowMap(unsigned int width, unsigned int height, const PCFFilter& pcf, float biasMultiplier, float shadowStrength = 0.0f);

		/**
		 * Resizes the cascades
		 */
		void resize(unsigned int width, unsigned int height);

		TransformShader* getDepthPass();

		unsigned getHeight() const;

		const PCFFilter& getPCF() const;

		nex::Texture* getRenderResult();
		const nex::Texture* getRenderResult() const;

		float getShadowStrength()const;

		unsigned getWidth() const;

		const glm::mat4& getView() const;
		const glm::mat4& getViewProjection() const;
		const glm::mat4& getProjection() const;
		

		void render(const nex::RenderCommandQueue::Buffer& shadowCommands);

		void setBiasMultiplier(float bias);

		void setPCF(const PCFFilter& filter);

		/**
		 * @param strength : a float in the range [0,1]
		 */
		void setShadowStrength(float strength);

		void update(const DirLight& dirLight, const AABB& shadowBounds);

	protected:

		class DepthPass : public TransformShader
		{
		public:
			DepthPass();
		};

		std::unique_ptr<DepthPass> mDepthPass;
		std::unique_ptr<RenderTarget> mRenderTarget;

		PCFFilter mPCF;
		float mBiasMultiplier;
		float mShadowStrength;
		glm::mat4 mView;
		glm::mat4 mProjection;
		glm::mat4 mViewProj;
	};


	class ShadowMap_ConfigurationView : public nex::gui::Drawable {
	public:
		ShadowMap_ConfigurationView(const nex::ShadowMap* model);

	protected:
		//void drawShadowStrengthConfig();
		//void drawCascadeNumConfig();
		//void drawCascadeBiasConfig();
		//void drawCascadeDimensionConfig();
		//void drawPCFConfig();
		void drawSelf() override;

	private:
		const nex::ShadowMap* mModel;
		nex::gui::TextureView mTextureView;
	};
}