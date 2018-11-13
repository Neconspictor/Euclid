#ifndef SHADING_MODEL_FACTORY_GL_HPP
#define SHADING_MODEL_FACTORY_GL_HPP

#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include "PBR_DeferredGL.hpp"

class ShadingModelFactoryGL {

public:
	ShadingModelFactoryGL(RendererOpenGL* renderer);
	virtual ~ShadingModelFactoryGL() = default;

	std::unique_ptr<PBR_DeferredGL> create_PBR_Deferred_Model(TextureGL* backgroundHDR);

private:
	RendererOpenGL* renderer;
};

#endif