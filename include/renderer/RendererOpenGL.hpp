#ifndef RENDERER_OPENGL_HPP
#define RENDERER_OPENGL_HPP
#include "renderer/Renderer.hpp"

class RendererOpenGL : public Renderer
{
public:
	RendererOpenGL();
	virtual ~RendererOpenGL();
	void init() override;
	void beginScene() override;
	void endScene() override;
	void present() override;
	void release() override;
	RendererType getType() override;
};

#endif
