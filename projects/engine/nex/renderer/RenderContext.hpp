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
		const Camera* camera = nullptr;
		const glm::mat4* proj = nullptr;
		const glm::mat4* view = nullptr;
		const glm::mat4* invViewProj = nullptr;

		CascadedShadow* csm = nullptr;
		GlobalIllumination* gi = nullptr;
		RenderTarget* irradianceAmbientReflection = nullptr;
		EffectLibrary* lib = nullptr;

		RenderTarget* out = nullptr;
		const Texture* outStencilView = nullptr;

		RenderTarget* pingPong = nullptr;
		const Texture* pingPongStencilView = nullptr;

		const DirLight* sun = nullptr;
		StencilTest* stencilTest = nullptr;

		float time;
		float frameTime;

		unsigned windowWidth;
		unsigned windowHeight;
	};
}