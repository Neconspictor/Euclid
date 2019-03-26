#pragma once

namespace nex
{
	class Camera;
	class SceneNode;
	class RenderBackend;

	class Renderer
	{
	public:

		Renderer(RenderBackend* renderBackend);
		virtual ~Renderer() = default;

		virtual void render(SceneNode* scene, Camera* camera, float frameTime, unsigned windowWidth, unsigned windowHeight) = 0;

	protected:
		RenderBackend* m_renderBackend;
	};
}