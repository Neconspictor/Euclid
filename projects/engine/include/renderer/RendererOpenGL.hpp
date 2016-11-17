#ifndef RENDERER_OPENGL_HPP
#define RENDERER_OPENGL_HPP
#include <platform/Renderer.hpp>
#include <texture/opengl/TextureManagerGL.hpp>


class RendererOpenGL : public Renderer
{
public:
	RendererOpenGL();
	virtual ~RendererOpenGL();
	void init() override;
	void beginScene() override;
	void endScene() override;
	TextureManager* getTextureManager() override;
	static TextureManagerGL* getTextureManagerGL();
	RendererType getType() const override;
	void present() override;
	void release() override;

};

#endif
