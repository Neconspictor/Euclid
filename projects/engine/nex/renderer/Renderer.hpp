#pragma once
#include "nex/math/Constant.hpp"

namespace nex
{
	class Camera;
	struct DirLight;
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
			const Camera&  camera, const DirLight& sun,
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