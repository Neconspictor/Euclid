#ifndef PBR_DEFERRED_HPP
#define PBR_DEFERRED_HPP

#include <shading_model/PBR.hpp>

class PBR_Deferred : public PBR {

public:
	PBR_Deferred(Renderer3D* renderer, Texture* backgroundHDR);
  virtual ~PBR_Deferred();

  virtual RenderTarget* createMultipleRenderTarget(int width, int height) = 0;
};

#endif