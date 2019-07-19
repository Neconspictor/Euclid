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

		virtual void render(const RenderCommandQueue& queue, 
			PerspectiveCamera* camera, 
			DirectionalLight* sun, 
			unsigned viewportWidth, 
			unsigned viewportHeight, 
			RenderTarget* out) = 0;

		PbrTechnique* getPbrTechnique();
		const PbrTechnique* getPbrTechnique() const;

	protected:
		PbrTechnique* mPbrTechnique;
	};
}