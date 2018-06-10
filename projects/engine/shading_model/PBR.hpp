#ifndef PBR_HPP
#define PBR_HPP

#include <texture/Texture.hpp>
#include <model/Vob.hpp>
#include<shader/PBRShader.hpp>
#include <scene/SceneNode.hpp>
#include <light/Light.hpp>
#include <sprite/Sprite.hpp>
#include<memory>

class Renderer3D;

class PBR {

public:
  PBR(Renderer3D* renderer, Texture* backgroundHDR);
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


	Renderer3D* renderer;
	PBRShader* shader;

	Sprite brdfSprite;
	Vob skybox;
};

#endif