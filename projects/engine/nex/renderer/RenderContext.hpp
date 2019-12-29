#pragma once

#include <glm/glm.hpp>

namespace nex
{
	class Camera;
	struct DirLight;
	class Texture;
	class RenderTarget;
	class EffectLibrary;
	class CascadedShadow;
	class GlobalIllumination;
	class StencilTest;


	struct RenderContext
	{
		const Camera* camera;
		const glm::mat4* proj;
		const glm::mat4* view;
		const glm::mat4* invViewProj;

		CascadedShadow* csm;
		GlobalIllumination* gi;
		RenderTarget* irradianceAmbientReflection;
		EffectLibrary* lib;

		RenderTarget* out;
		const Texture* outStencilView;

		RenderTarget* pingPong;
		const Texture* pingPongStencilView;

		const DirLight* sun;
		StencilTest* stencilTest;

		float time;
		float frameTime;

		unsigned windowWidth;
		unsigned windowHeight;
	};
}