#ifndef PBR_HPP
#define PBR_HPP

#include <texture/Texture.hpp>
#include <renderer/Renderer3D.hpp>
#include <model/Vob.hpp>
#include<shader/PBRShader.hpp>
#include <scene/SceneNode.hpp>
#include <light/Light.hpp>

class PBR {

public:
  PBR();
  virtual ~PBR();

  void init(Renderer3D* renderer, Texture* backgroundHDR);

  void drawSceneToShadowMap(SceneNode * scene, 
	  float frameTimeElapsed,
	  DepthMap* shadowMap,
	  const DirectionalLight& light,
	  const glm::mat4& lightViewMatrix,
	  const glm::mat4& lightProjMatrix);

  void drawScene(SceneNode * scene,
	  RenderTarget* renderTarget,
	  const glm::vec3& cameraPosition,
	  float frameTimeElapsed,
	  Texture* shadowMap,
	  const DirectionalLight& light,
	  const glm::mat4& lightViewMatrix,
	  const glm::mat4& lightProjMatrix,
	  const glm::mat4& view,
	  const glm::mat4& projection);

  void drawSky(RenderTarget* renderTarget, 
	  const glm::mat4& projection, 
	  const glm::mat4& view);


  CubeMap* getConvolutedEnvironmentMap();

  CubeMap* getEnvironmentMap();


private:

	CubeMap* renderBackgroundToCube(Texture* background);
	CubeMap* convolute(CubeMap* source);



	CubeMap* convolutedEnvironmentMap;
	CubeMap* environmentMap;
	Renderer3D* renderer;
	PBRShader* shader;
	Vob skybox;
};

#endif