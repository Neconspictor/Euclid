#pragma once
#include "nex/math/Constant.hpp"
#include <functional>
#include <unordered_map>

namespace nex
{
	class Camera;
	struct DirLight;
	class RenderCommandQueue;
	class PerspectiveCamera;
	class RenderTarget;
	class PbrTechnique;
	class Texture;

	class Renderer
	{
	public:

		Renderer(PbrTechnique* pbrTechnique);

		virtual ~Renderer();

		unsigned getWidth()const;
		unsigned getHeight()const;

		virtual void render(const RenderCommandQueue& queue, 
			const Camera&  camera, const DirLight& sun,
			unsigned viewportWidth, 
			unsigned viewportHeight, 
			bool postProcess,
			RenderTarget* out) = 0;

		virtual void updateRenderTargets(unsigned width, unsigned height);

		PbrTechnique* getPbrTechnique();
		const PbrTechnique* getPbrTechnique() const;

		const std::vector<std::string>& getRenderLayerDescriptions();

		Texture* getActiveRenderLayer();

		virtual RenderTarget* getTempRendertTarget() = 0;

		virtual void pushDepthFunc(std::function<void()> func) = 0;

		void setActiveRenderLayer(const std::string& desc);

	protected:
		PbrTechnique* mPbrTechnique;
		unsigned mWidth;
		unsigned mHeight;
		std::vector<std::string> mRenderLayerDescs;
		std::unordered_map<std::string, std::function<Texture* ()>> mRenderlayers;
		std::function<Texture*()> mActiveRenderLayerProvider;
	};
}