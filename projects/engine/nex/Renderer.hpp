#pragma once

namespace nex
{
	class Camera;
	class SceneNode;
	class RenderBackend;
	class DirectionalLight;

	class Renderer
	{
	public:

		Renderer(RenderBackend* renderBackend);
		virtual ~Renderer() = default;

		virtual void render(SceneNode* scene, Camera* camera, DirectionalLight* sun, float frameTime, unsigned windowWidth, unsigned windowHeight) = 0;

	protected:
		RenderBackend* m_renderBackend;
	};
}