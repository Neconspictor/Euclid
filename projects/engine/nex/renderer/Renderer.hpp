#pragma once
#include "nex/math/Constant.hpp"
#include <functional>
#include <unordered_map>
#include <nex/shader/Shader.hpp>

namespace nex
{
	class RenderCommandQueue;
	class PerspectiveCamera;
	class RenderTarget;
	class PbrTechnique;
	class Texture;
	class SpriteShader;

	class Renderer
	{
	public:

		struct RenderLayer {
			std::string desc; 
			std::function<Texture* ()> textureProvider;
			std::function<SpriteShader * ()> spriteShaderProvider = [=]() {return nullptr; };
		};

		Renderer(PbrTechnique* pbrTechnique);

		virtual ~Renderer();

		unsigned getWidth()const;
		unsigned getHeight()const;

		virtual void render(const RenderCommandQueue& queue, 
			const RenderContext& constants,
			bool postProcess,
			RenderTarget* out = nullptr) = 0;

		virtual void updateRenderTargets(unsigned width, unsigned height);

		PbrTechnique* getPbrTechnique();
		const PbrTechnique* getPbrTechnique() const;

		const std::vector<RenderLayer>& getRenderLayers();

		size_t getRenderLayerIndexByName(const std::string& desc) const;
		size_t getActiveRenderLayer() const;


		virtual RenderTarget* getActiveIrradianceAmbientReflectionRT() = 0;
		
		virtual RenderTarget* getOutRT() = 0;
		virtual const Texture* getOutStencilView() = 0;

		virtual RenderTarget* getPingPongRT() = 0;
		virtual const Texture* getPingPongStencilView() = 0;
		

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