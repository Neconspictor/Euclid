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

		virtual void render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight) = 0;

	protected:
		RenderBackend* m_renderBackend;
	};
}