#pragma once


#include <nex/texture/RenderTarget.hpp>
#include <nex/mesh/Vob.hpp>
#include <nex/shader/PBRShader.hpp>
#include <nex/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/texture/Sprite.hpp>
#include <nex/texture/Image.hpp>


namespace nex
{
	class RenderBackend;

	class PBR {

	public:
		PBR(RenderBackend* renderer, Texture* backgroundHDR);
		virtual ~PBR();

		virtual void drawSceneToShadowMap(SceneNode * scene,
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


		CubeMap* getConvolutedEnvironmentMap();

		CubeMap* getEnvironmentMap();

		CubeMap* getPrefilteredEnvironmentMap();

		Texture* getBrdfLookupTexture();

		StoreImage readBrdfLookupPixelData() const;
		StoreImage readBackgroundPixelData() const;


	protected:

		StoreImage readConvolutedEnvMapPixelData();
		StoreImage readPrefilteredEnvMapPixelData();
		void init(Texture* backgroundHDR);

		CubeMap* renderBackgroundToCube(Texture* background);
		CubeMap* convolute(CubeMap* source);
		CubeMap* prefilter(CubeMap* source);
		Texture2D* createBRDFlookupTexture();

		CubeMap* convolutedEnvironmentMap;
		CubeMap* prefilteredEnvMap;
		CubeMap* environmentMap;
		Texture2D* brdfLookupTexture;


		RenderBackend* renderer;

		Sprite brdfSprite;
		Vob skybox;
	};
}