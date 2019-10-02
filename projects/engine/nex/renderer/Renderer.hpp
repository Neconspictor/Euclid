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

		struct RenderLayer {
			std::string desc; 
			std::function<Texture* ()> textureProvider;
		};

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

		const std::vector<RenderLayer>& getRenderLayers();

		size_t getRenderLayerIndexByName(const std::string& desc) const;
		size_t getActiveRenderLayer() const;

		virtual RenderTarget* getOutRendertTarget() = 0;

		virtual void pushDepthFunc(std::function<void()> func) = 0;

		void setActiveRenderLayer(size_t index);

	protected:
		PbrTechnique* mPbrTechnique;
		unsigned mWidth;
		unsigned mHeight;
		std::vector<RenderLayer> mRenderLayers;
		size_t mActiveRenderLayer;
	};
}