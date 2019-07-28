#pragma once
#include "nex/math/Constant.hpp"

namespace nex
{
	class Camera;
	class DirectionalLight;
	class RenderCommandQueue;
	class PerspectiveCamera;
	class RenderTarget;
	class PbrTechnique;

	class Renderer
	{
	public:

		Renderer(PbrTechnique* pbrTechnique);

		virtual ~Renderer() = default;

		unsigned getWidth()const;
		unsigned getHeight()const;

		virtual void render(const RenderCommandQueue& queue, 
			const Camera&  camera, const DirectionalLight& sun,
			unsigned viewportWidth, 
			unsigned viewportHeight, 
			bool postProcess,
			RenderTarget* out) = 0;

		virtual void updateRenderTargets(unsigned width, unsigned height);

		PbrTechnique* getPbrTechnique();
		const PbrTechnique* getPbrTechnique() const;

	protected:
		PbrTechnique* mPbrTechnique;
		unsigned mWidth;
		unsigned mHeight;
	};
}