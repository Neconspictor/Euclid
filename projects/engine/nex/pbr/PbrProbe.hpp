#pragma once

#include <nex/texture/Sprite.hpp>
#include <nex/texture/Image.hpp>
#include "PbrPass.hpp"


namespace nex
{
	class FileSystem;
	class SceneNode;
	class DirectionalLight;
	class StaticMeshContainer;
	class SphereMesh;


	class PbrProbeFactory
	{
	public:

		static PbrProbeFactory* get(const std::filesystem::path& probeCompiledDirectory);

		std::unique_ptr<PbrProbe> create(Texture* backgroundHDR, unsigned probeID);

	private:
		PbrProbeFactory(const std::filesystem::path& probeCompiledDirectory);
		std::unique_ptr<FileSystem> mFileSystem;
	};

	class PbrProbe {

	public:
		PbrProbe(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

		~PbrProbe();

		//void drawSky(const glm::mat4& projection,
		//	const glm::mat4& view);

		static void initGlobals(const std::filesystem::path& probeRoot);


		CubeMap* getConvolutedEnvironmentMap() const;

		CubeMap* getEnvironmentMap() const;

		CubeMap* getPrefilteredEnvironmentMap() const;

		static Texture2D* getBrdfLookupTexture();
		StoreImage readBackgroundPixelData() const;


	protected:

		class ProbeTechnique;
		class ProbeMaterial;

		static std::shared_ptr<Texture2D> createBRDFlookupTexture(Pass* brdfPrecompute);
		static StoreImage readBrdfLookupPixelData();

		StoreImage readConvolutedEnvMapPixelData();
		StoreImage readPrefilteredEnvMapPixelData();
		void init(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

		std::shared_ptr<CubeMap> renderBackgroundToCube(Texture* background);
		std::shared_ptr<CubeMap> convolute(CubeMap* source);
		std::shared_ptr<CubeMap> prefilter(CubeMap* source);

		std::shared_ptr<CubeMap> convolutedEnvironmentMap;
		std::shared_ptr<CubeMap> prefilteredEnvMap;
		std::shared_ptr<CubeMap> environmentMap;

		std::unique_ptr<PbrConvolutionPass> mConvolutionPass;
		std::unique_ptr<PbrPrefilterPass> mPrefilterPass;

		static std::shared_ptr<Texture2D> mBrdfLookupTexture;
		static std::unique_ptr<ProbeTechnique> mTechnique;
		static std::unique_ptr<SphereMesh> mMesh;

		std::unique_ptr<ProbeMaterial> mMaterial;

		StaticMeshContainer* mSkyBox;
	};
}
