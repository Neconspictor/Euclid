#ifndef RENDERER_OPENGL_HPP
#define RENDERER_OPENGL_HPP
#include <texture/opengl/TextureManagerGL.hpp>
#include <renderer/Renderer3D.hpp>


class RendererOpenGL : public Renderer3D
{
public:
	RendererOpenGL();
	
	virtual ~RendererOpenGL();
	
	void init() override;
	
	void beginScene() override;
	
	void endScene() override;

	ShaderManager* getShaderManager() override;
	
	TextureManager* getTextureManager() override;
	
	static TextureManagerGL* getTextureManagerGL();
	
	RendererType getType() const override;
	
	void present() override;
	
	void release() override;
};

#endif
