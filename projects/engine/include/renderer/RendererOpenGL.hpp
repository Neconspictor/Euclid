#ifndef RENDERER_OPENGL_HPP
#define RENDERER_OPENGL_HPP
#include <renderer/Renderer3D.hpp>


class RendererOpenGL : public Renderer3D
{
public:
	RendererOpenGL();
	
	virtual ~RendererOpenGL();
	
	void init() override;
	
	void beginScene() override;
	
	void endScene() override;

	ModelDrawer* getModelDrawer() override;

	ModelManager* getModelManager() override;

	ShaderManager* getShaderManager() override;
	
	TextureManager* getTextureManager() override;
	
	RendererType getType() const override;
	
	void present() override;
	
	void release() override;

	void setViewPort(int x, int y, int width, int height) override;

protected:
	/**
	 * A function for checking any opengl related errors.
	 * Mainly intended for debug purposes
	 * NOTE: Throws an OpenglException if any opengl related error occured
	 * since the last call of glGetError()
	 * @param errorPrefix: a prefix that will be put in front of the OpenglException.
	 */
	void checkGLErrors(std::string errorPrefix) const;
};

#endif
