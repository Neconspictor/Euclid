#ifndef PBR_DEFERRED_HPP
#define PBR_DEFERRED_HPP

#include <nex/shading_model/PBR.hpp>

class PBR_Deferred : public PBR {

public:
	PBR_Deferred(RenderBackend* renderer, Texture* backgroundHDR);

   virtual std::unique_ptr<PBR_GBuffer>createMultipleRenderTarget(int width, int height) = 0;

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

private:
	Sprite screenSprite;
};

#endif