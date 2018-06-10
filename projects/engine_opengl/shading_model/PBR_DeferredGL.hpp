#ifndef PBR_DEFERRED_GL_HPP
#define PBR_DEFERRED_GL_HPP

#include <shading_model/PBR_Deferred.hpp>
#include <list>
#include <texture/TextureGL.hpp>

class PBR_DeferredGL : public PBR_Deferred {

public:
	PBR_DeferredGL(Renderer3D* renderer, Texture* backgroundHDR);
  virtual ~PBR_DeferredGL();

  virtual void drawGeometryScene(SceneNode * scene,
	  const glm::mat4& view,
	  const glm::mat4& projection);

  virtual void drawLighting(SceneNode * scene,
	  PBR_GBuffer* gBuffer,
	  Texture* shadowMap,
	  Texture* ssaoMap,
	  const DirectionalLight& light,
	  const glm::mat4& viewFromGPass,
	  const glm::mat4& worldToLight);

  virtual void drawSky(const glm::mat4& projection,
	  const glm::mat4& view);

  virtual std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height) override;
};

#endif