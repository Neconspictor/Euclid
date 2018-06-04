#ifndef PBR_DEFERRED_GL_HPP
#define PBR_DEFERRED_GL_HPP

#include <shading_model/PBR_Deferred.hpp>
#include <list>
#include <texture/TextureGL.hpp>

class PBR_DeferredGL : public PBR_Deferred {

public:
	PBR_DeferredGL(Renderer3D* renderer, Texture* backgroundHDR);
  virtual ~PBR_DeferredGL();

  virtual std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height) override;
};

#endif