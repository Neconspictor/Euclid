#ifndef PBR_HPP
#define PBR_HPP

#include <nex/texture/Texture.hpp>
#include <nex/model/Vob.hpp>
#include <nex/shader/PBRShader.hpp>
#include <nex/scene/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/sprite/Sprite.hpp>
#include<memory>

class RenderBackend;

class PBR {

public:
  PBR(RenderBackend* renderer, Texture* backgroundHDR);
  virtual ~PBR();

  virtual void drawSceneToShadowMap(SceneNode * scene,
	  DepthMap* shadowMap,
	  const DirectionalLight& light,
	  const glm::mat4& lightViewMatrix,
	  const glm::mat4& lightProjMatrix);

  virtual void drawScene(SceneNode * scene,
	  const glm::vec3& cameraPosition,
	  Texture* shadowMap,
	  const DirectionalLight& light,
	  const glm::mat4& lightViewMatrix,
	  const glm::mat4& lightProjMatrix,
	  const glm::mat4& view,
	  const glm::mat4& projection);

  virtual void drawSky(const glm::mat4& projection, 
	  const glm::mat4& view);


  virtual CubeMap* getConvolutedEnvironmentMap();

  virtual CubeMap* getEnvironmentMap();

  virtual CubeMap* getPrefilteredEnvironmentMap();

  virtual Texture* getBrdfLookupTexture();


protected:

	virtual void init(Texture* backgroundHDR);

	CubeRenderTarget* renderBackgroundToCube(Texture* background);
	CubeRenderTarget* convolute(CubeMap* source);
	CubeRenderTarget* prefilter(CubeMap* source);
	RenderTarget* createBRDFlookupTexture();

	CubeRenderTarget* convolutedEnvironmentMap;
	CubeRenderTarget* prefilterRenderTarget;
	CubeRenderTarget* environmentMap;
	RenderTarget* brdfLookupTexture;


	RenderBackend* renderer;
	PBRShader* shader;

	Sprite brdfSprite;
	Vob skybox;
};

#endif