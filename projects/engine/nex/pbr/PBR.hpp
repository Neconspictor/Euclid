#pragma once

#include <nex/mesh/Vob.hpp>
#include <nex/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/texture/Sprite.hpp>
#include <nex/texture/Image.hpp>
#include "nex/texture/RenderTarget.hpp"
#include "nex/shader/PBRShader.hpp"


namespace nex
{
	class PBR {

	public:
		PBR(Texture* backgroundHDR);
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


		CubeMap* getConvolutedEnvironmentMap() const;

		CubeMap* getEnvironmentMap() const;

		CubeMap* getPrefilteredEnvironmentMap() const;

		Texture2D* getBrdfLookupTexture() const;

		StoreImage readBrdfLookupPixelData() const;
		StoreImage readBackgroundPixelData() const;


	protected:

		StoreImage readConvolutedEnvMapPixelData();
		StoreImage readPrefilteredEnvMapPixelData();
		void init(Texture* backgroundHDR);

		std::shared_ptr<CubeMap> renderBackgroundToCube(Texture* background);
		std::shared_ptr<CubeMap> convolute(CubeMap* source);
		std::shared_ptr<CubeMap> prefilter(CubeMap* source);
		std::shared_ptr<Texture2D> createBRDFlookupTexture();

		std::shared_ptr<CubeMap> convolutedEnvironmentMap;
		std::shared_ptr<CubeMap> prefilteredEnvMap;
		std::shared_ptr<CubeMap> environmentMap;
		std::shared_ptr<Texture2D> brdfLookupTexture;

		std::unique_ptr<PBR_ConvolutionShader> mConvolutionPass;
		std::unique_ptr<PBR_PrefilterShader> mPrefilterPass;
		std::unique_ptr<PBR_BrdfPrecomputeShader> mBrdfPrecomputePass;

		std::unique_ptr<PBRShader> mForwardShader;

		Sprite brdfSprite;
		Vob skybox;
	};
}
