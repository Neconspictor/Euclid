#ifndef SHADING_MODEL_FACTORY_GL_HPP
#define SHADING_MODEL_FACTORY_GL_HPP

#include <shading_model/ShadingModelFactory.hpp>

class ShadingModelFactoryGL : public ShadingModelFactory {

public:
	ShadingModelFactoryGL(Renderer3D* renderer);
	virtual ~ShadingModelFactoryGL() {
		int i = 0;
	};

	virtual std::unique_ptr<PBR_Deferred> create_PBR_Deferred_Model(Texture* backgroundHDR) override;
};

#endif