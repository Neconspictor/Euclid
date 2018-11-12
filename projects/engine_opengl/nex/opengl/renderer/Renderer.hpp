#pragma once

#include <nex/opengl/renderer/RendererOpenGL.hpp>


class Camera;
class SceneNode;

class Renderer
{
public:
	
	Renderer(RendererOpenGL* renderBackend);
	virtual ~Renderer() = default;

	virtual void render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight) = 0;

protected:
	RendererOpenGL* m_renderBackend;
};