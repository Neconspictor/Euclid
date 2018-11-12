#ifndef PBR_DEFERRED_GL_HPP
#define PBR_DEFERRED_GL_HPP

#include <list>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/opengl/texture/SamplerGL.hpp>
#include "nex/gui/Drawable.hpp"
#include <nex/opengl/shading_model/PBR.hpp>


class PBR_DeferredGL : public PBR {

public:
	PBR_DeferredGL(RendererOpenGL* renderer, Texture* backgroundHDR);
  virtual ~PBR_DeferredGL();

  void drawGeometryScene(SceneNode * scene,
	  const glm::mat4& view,
	  const glm::mat4& projection);

  void drawLighting(SceneNode * scene,
	  PBR_GBuffer* gBuffer,
	  Texture* shadowMap,
	  Texture* ssaoMap,
	  const DirectionalLight& light,
	  const glm::mat4& viewFromGPass,
	  const glm::mat4& worldToLight,
	  CascadedShadow::CascadeData* cascadeData,
	  Texture* cascadedDepthMap);

  void drawSky(const glm::mat4& projection,
	  const glm::mat4& view);

  std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height);

private:
	Sprite screenSprite;
};

class PBR_Deferred_ConfigurationView : public nex::engine::gui::Drawable {
public:
	PBR_Deferred_ConfigurationView(PBR_DeferredGL* pbr);

protected:
	void drawSelf() override;

private:
	PBR_DeferredGL * m_pbr;
};

#endif