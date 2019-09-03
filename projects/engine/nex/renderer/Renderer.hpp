#pragma once
#include "nex/math/Constant.hpp"
#include <functional>

namespace nex
{
	class Camera;
	struct DirLight;
	class RenderCommandQueue;
	class PerspectiveCamera;
	class RenderTarget;
	class PbrTechnique;
	class ProbeCluster;

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

		ProbeCluster* getProbeCluster();
		const ProbeCluster* getProbeCluster() const;

		virtual void pushDepthFunc(std::function<void()> func) = 0;

	protected:
		PbrTechnique* mPbrTechnique;
		std::unique_ptr<ProbeCluster> mProbeCluster;
		unsigned mWidth;
		unsigned mHeight;
	};
}