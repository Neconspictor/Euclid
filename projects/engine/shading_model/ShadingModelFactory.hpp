#ifndef SHADING_MODEL_FACTORY_HPP
#define SHADING_MODEL_FACTORY_HPP

#include <shading_model/PBR.hpp>
#include <shading_model/PBR_Deferred.hpp>
#include<memory>

class ShadingModelFactory {

public:
	ShadingModelFactory(Renderer3D* renderer);
	virtual ~ShadingModelFactory() {};

	virtual std::unique_ptr<PBR> create_PBR_Model(Texture* backgroundHDR);
	virtual std::unique_ptr<PBR_Deferred> create_PBR_Deferred_Model(Texture* backgroundHDR) = 0;

protected:
	Renderer3D* renderer;
};

#endif