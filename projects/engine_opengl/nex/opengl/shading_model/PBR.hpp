#ifndef PBR_HPP
#define PBR_HPP

#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/model/Vob.hpp>
#include <nex/opengl/shader/PBRShaderGL.hpp>
#include <nex/opengl/scene/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/opengl/texture/Sprite.hpp>

class RendererOpenGL;

class PBR {

public:
  PBR(RendererOpenGL* renderer, TextureGL* backgroundHDR);
  virtual ~PBR();

  virtual void drawSceneToShadowMap(SceneNode * scene,
	  DepthMapGL* shadowMap,
	  const DirectionalLight& light,
	  const glm::mat4& lightViewMatrix,
	  const glm::mat4& lightProjMatrix);

  virtual void drawScene(SceneNode * scene,
	  const glm::vec3& cameraPosition,
	  TextureGL* shadowMap,
	  const DirectionalLight& light,
	  const glm::mat4& lightViewMatrix,
	  const glm::mat4& lightProjMatrix,
	  const glm::mat4& view,
	  const glm::mat4& projection);

  virtual void drawSky(const glm::mat4& projection, 
	  const glm::mat4& view);


  CubeMapGL* getConvolutedEnvironmentMap();

  CubeMapGL* getEnvironmentMap();

  CubeMapGL* getPrefilteredEnvironmentMap();

  TextureGL* getBrdfLookupTexture();


protected:

	static void vobRenderCallback(const Vob* vob, const ModelDrawerGL::RenderContext* context);
	static void meshRenderCallback(const MeshGL* mesh, const ModelDrawerGL::RenderContext* context);

	void init(TextureGL* backgroundHDR);

	CubeRenderTargetGL* renderBackgroundToCube(TextureGL* background);
	CubeRenderTargetGL* convolute(CubeMapGL* source);
	CubeRenderTargetGL* prefilter(CubeMapGL* source);
	RenderTargetGL* createBRDFlookupTexture();

	CubeRenderTargetGL* convolutedEnvironmentMap;
	CubeRenderTargetGL* prefilterRenderTarget;
	CubeRenderTargetGL* environmentMap;
	RenderTargetGL* brdfLookupTexture;


	RendererOpenGL* renderer;
	PBRShaderGL* shader;

	Sprite brdfSprite;
	Vob skybox;
};

#endif