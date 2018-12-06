#pragma once
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/texture/Texture.hpp>

namespace nex
{
	class SMAA_GL
	{
	public:
		explicit SMAA_GL(RendererOpenGL* renderer);
		virtual ~SMAA_GL();

		void antialias(RenderTarget* renderTarget);

		void init();

		void reset();

		void updateBuffers();

	private:
		RendererOpenGL* renderer;
		RenderTarget* edgesTex;
		RenderTarget* blendTex;

		Texture* areaTex;
		Texture* searchTex;

		GLuint edgeDetectionPass;
		GLuint blendingWeightCalculationPass;
		GLuint neighborhoodBlendingPass;

		bool initialized;
	};
}