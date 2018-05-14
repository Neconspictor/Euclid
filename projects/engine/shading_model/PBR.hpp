#ifndef PBR_HPP
#define PBR_HPP

#include <texture/Texture.hpp>
#include <renderer/Renderer3D.hpp>
#include <model/Vob.hpp>
#include<shader/PBRShader.hpp>
#include <scene/SceneNode.hpp>
#include <light/Light.hpp>
#include <sprite/Sprite.hpp>

class PBR {

public:
  PBR();
  virtual ~PBR();

  virtual void init(Renderer3D* renderer, Texture* backgroundHDR);

  virtual void drawSceneToShadowMap(SceneNode * scene, 
	  float frameTimeElapsed,
	  DepthMap* shadowMap,
	  const DirectionalLight& light,
	  const glm::mat4& lightViewMatrix,
	  const glm::mat4& lightProjMatrix);

  virtual void drawScene(SceneNode * scene,
	  RenderTarget* renderTarget,
	  const glm::vec3& cameraPosition,
	  float frameTimeElapsed,
	  Texture* shadowMap,
	  const DirectionalLight& light,
	  const glm::mat4& lightViewMatrix,
	  const glm::mat4& lightProjMatrix,
	  const glm::mat4& view,
	  const glm::mat4& projection);

  virtual void drawSky(RenderTarget* renderTarget,
	  const glm::mat4& projection, 
	  const glm::mat4& view);


  virtual CubeMap* getConvolutedEnvironmentMap();

  virtual CubeMap* getEnvironmentMap();

  virtual CubeMap* getPrefilteredEnvironmentMap();

  virtual Texture* getBrdfLookupTexture();


protected:

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