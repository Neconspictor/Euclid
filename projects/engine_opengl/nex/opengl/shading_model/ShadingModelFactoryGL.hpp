#ifndef SHADING_MODEL_FACTORY_GL_HPP
#define SHADING_MODEL_FACTORY_GL_HPP

#include <nex/opengl/shading_model/PBR_DeferredGL.hpp>
#include <memory>

class RendererOpenGL;

class ShadingModelFactoryGL {

public:
	ShadingModelFactoryGL(RendererOpenGL* renderer);
	virtual ~ShadingModelFactoryGL() = default;

	std::unique_ptr<PBR_DeferredGL> create_PBR_Deferred_Model(TextureGL* backgroundHDR);

private:
	RendererOpenGL* renderer;
};

#endif