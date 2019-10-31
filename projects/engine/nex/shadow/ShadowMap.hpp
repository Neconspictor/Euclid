#pragma once
#include <nex/texture/Texture.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/texture/RenderTarget.hpp>
#include "nex/gui/Drawable.hpp"
#include <nex/shader/Pass.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/shadow/ShadowCommon.hpp>

namespace nex
{

	struct DirLight;

	class ShadowMap
	{
	public:

		ShadowMap(unsigned int width, unsigned int height, const PCFFilter& pcf, float biasMultiplier, float shadowStrength = 0.0f);

		nex::Texture* getDepthTexture();

		/**
		 * Resizes the cascades
		 */
		void resize(unsigned int width, unsigned int height);

		TransformPass* getDepthPass();

		unsigned getHeight() const;

		const PCFFilter& getPCF() const;

		nex::Texture* getRenderResult();
		const nex::Texture* getRenderResult() const;

		float getShadowStrength()const;

		unsigned getWidth() const;

		const glm::mat4& getView() const;
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

		class DepthPass : public TransformPass
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
	};
}