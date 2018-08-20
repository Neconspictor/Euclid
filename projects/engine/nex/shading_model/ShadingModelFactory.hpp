#ifndef SHADING_MODEL_FACTORY_HPP
#define SHADING_MODEL_FACTORY_HPP

#include <nex/shading_model/PBR.hpp>
#include <nex/shading_model/PBR_Deferred.hpp>
#include<memory>

class ShadingModelFactory {

public:
	ShadingModelFactory(RenderBackend* renderer);
	virtual ~ShadingModelFactory() {};

	virtual std::unique_ptr<PBR> create_PBR_Model(Texture* backgroundHDR);
	virtual std::unique_ptr<PBR_Deferred> create_PBR_Deferred_Model(Texture* backgroundHDR) = 0;

protected:
	RenderBackend* renderer;
};

#endif