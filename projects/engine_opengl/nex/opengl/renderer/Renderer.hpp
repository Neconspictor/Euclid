#pragma once

#include <nex/renderer/RenderBackend.hpp>


class Camera;
class SceneNode;

class Renderer
{
public:

	using Backend = RenderBackend*;
	
	Renderer(Backend backend);
	virtual ~Renderer() = default;

	virtual void render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight) = 0;

protected:
	Backend m_renderBackend;
};