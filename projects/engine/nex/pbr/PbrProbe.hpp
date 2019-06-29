#pragma once

#include <nex/texture/Sprite.hpp>
#include <nex/texture/Image.hpp>
#include "PbrPass.hpp"
#include <nex/Scene.hpp>


namespace nex
{
	class FileSystem;
	class SceneNode;
	class DirectionalLight;
	class StaticMeshContainer;
	class SphereMesh;
	class Mesh;


	class PbrProbeFactory
	{
	public:

		static PbrProbeFactory* get(const std::filesystem::path& probeCompiledDirectory);

		//std::unique_ptr<PbrProbe> create(Texture* backgroundHDR, unsigned probeID);

	private:
		PbrProbeFactory(const std::filesystem::path& probeCompiledDirectory);
		std::unique_ptr<FileSystem> mFileSystem;
	};

	class PbrProbe {

	public:
		PbrProbe();

		PbrProbe& operator=(PbrProbe&&) = default;

		~PbrProbe();

		//void drawSky(const glm::mat4& projection,
		//	const glm::mat4& view);

		static void initGlobals(const std::filesystem::path& probeRoot);
		static Mesh* getSphere();


		CubeMap* getConvolutedEnvironmentMap() const;

		CubeMap* getEnvironmentMap() const;

		Material* getMaterial();

		CubeMap* getPrefilteredEnvironmentMap() const;

		static Texture2D* getBrdfLookupTexture();
		StoreImage readBackgroundPixelData() const;

		void initBackground(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

		void initPrefiltered(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

		void initIrradiance(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);
		void loadIrradianceFile(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);
		void createIrradianceTex(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

		void init(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

	protected:

		class ProbeTechnique;
		class ProbeMaterial;

		static std::unique_ptr<StaticMeshContainer> createSkyBox();
		static std::shared_ptr<Texture2D> createBRDFlookupTexture(Pass* brdfPrecompute);
		static StoreImage readBrdfLookupPixelData();

		StoreImage readConvolutedEnvMapPixelData();
		StoreImage readPrefilteredEnvMapPixelData();
		

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
		StoreImage mReadImage;
	};

	class ProbeVob : public Vob
	{
	public:
		ProbeVob(SceneNode* meshRootNode, PbrProbe* probe);
		virtual ~ProbeVob() = default;

		PbrProbe* getProbe();

	private:
		PbrProbe* mProbe;
	};
}