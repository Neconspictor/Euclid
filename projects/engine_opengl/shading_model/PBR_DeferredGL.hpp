#ifndef PBR_DEFERRED_GL_HPP
#define PBR_DEFERRED_GL_HPP

#include <shading_model/PBR_Deferred.hpp>

class PBR_DeferredGL : public PBR_Deferred {

public:
	PBR_DeferredGL(Renderer3D* renderer, Texture* backgroundHDR);
  virtual ~PBR_DeferredGL();

  virtual RenderTarget* createMultipleRenderTarget(int width, int height) override;

protected:
};

#endif