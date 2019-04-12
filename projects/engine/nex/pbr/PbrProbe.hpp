#pragma once

#include <nex/mesh/Vob.hpp>
#include <nex/texture/Sprite.hpp>
#include <nex/texture/Image.hpp>
#include "nex/shader/PBRShader.hpp"


namespace nex
{
	class SceneNode;
	class DirectionalLight;

	class PbrProbe {

	public:
		PbrProbe(Texture* backgroundHDR);

		//void drawSky(const glm::mat4& projection,
		//	const glm::mat4& view);


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

		std::unique_ptr<PbrConvolutionShader> mConvolutionPass;
		std::unique_ptr<PbrPrefilterShader> mPrefilterPass;
		std::unique_ptr<PbrBrdfPrecomputeShader> mBrdfPrecomputePass;

		Sprite brdfSprite;
		Vob skybox;
	};
}
