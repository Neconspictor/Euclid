#pragma once

#include <nex/mesh/Vob.hpp>
#include <nex/texture/Sprite.hpp>
#include <nex/texture/Image.hpp>
#include "PbrPass.hpp"


namespace nex
{
	class FileSystem;
	class SceneNode;
	class DirectionalLight;


	class PbrProbeFactory
	{
	public:

		PbrProbeFactory(std::filesystem::path probeCompiledDirectory);
		std::unique_ptr<PbrProbe> create(Texture* backgroundHDR, unsigned probeID);

	private:
		std::unique_ptr<FileSystem> mFileSystem;
	};

	class PbrProbe {

	public:
		PbrProbe(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

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
		void init(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

		std::shared_ptr<CubeMap> renderBackgroundToCube(Texture* background);
		std::shared_ptr<CubeMap> convolute(CubeMap* source);
		std::shared_ptr<CubeMap> prefilter(CubeMap* source);
		std::shared_ptr<Texture2D> createBRDFlookupTexture();

		std::shared_ptr<CubeMap> convolutedEnvironmentMap;
		std::shared_ptr<CubeMap> prefilteredEnvMap;
		std::shared_ptr<CubeMap> environmentMap;
		std::shared_ptr<Texture2D> brdfLookupTexture;

		std::unique_ptr<PbrConvolutionPass> mConvolutionPass;
		std::unique_ptr<PbrPrefilterPass> mPrefilterPass;
		std::unique_ptr<PbrBrdfPrecomputePass> mBrdfPrecomputePass;

		Sprite mBrdfSprite;
		StaticMeshContainer* mSkyBox;
	};
}
