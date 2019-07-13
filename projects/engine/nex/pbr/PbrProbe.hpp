#pragma once

#include <nex/texture/Sprite.hpp>
#include <nex/texture/Image.hpp>
#include "PbrPass.hpp"
#include <nex/Scene.hpp>
#include <nex/resource/Resource.hpp>
#include <memory>


namespace nex
{
	class FileSystem;
	class SceneNode;
	class DirectionalLight;
	class StaticMeshContainer;
	class SphereMesh;
	class Mesh;
	class CubeMapArray;
	class Sampler;

	class PbrProbeFactory
	{
	public:

		static constexpr unsigned IRRADIANCE_SIDE = 32;
		static const TextureData BRDF_DATA;
		static const TextureData IRRADIANCE_DATA;
		static const TextureData PREFILTERED_DATA;
		static const TextureData SOURCE_DATA;

		PbrProbeFactory(unsigned prefilteredSideWidth, unsigned prefilteredSideHeight, unsigned mapSize);

		static void init(const std::filesystem::path& probeCompiledDirectory, std::string probeFileExtension);

		/**
		 * Non blocking init function for probes.
		 */
		void initProbe(PbrProbe* probe, Texture* backgroundHDR, unsigned probeID);
		
	private:
		static std::unique_ptr<FileSystem> mFileSystem;
		std::unique_ptr<CubeMapArray> mIrraddianceMaps;
		std::unique_ptr<CubeMapArray> mPrefilteredMaps;
		
	};

	class PbrProbe : public Resource {

	public:

		struct Handles {
			uint64_t convoluted = 0;
			uint64_t prefiltered = 0;
		};

		PbrProbe();

		PbrProbe(PbrProbe&& o) noexcept = delete;
		PbrProbe& operator=(PbrProbe&& o) noexcept = delete;
		PbrProbe(const PbrProbe& o) noexcept = delete;
		PbrProbe& operator=(const PbrProbe& o) noexcept = delete;

		virtual ~PbrProbe();

		//void drawSky(const glm::mat4& projection,
		//	const glm::mat4& view);

		void createHandles();
		void activateHandles();
		void deactivateHandles();

		static void initGlobals(const std::filesystem::path& probeRoot);
		static Mesh* getSphere();


		CubeMap* getConvolutedEnvironmentMap() const;

		CubeMap* getEnvironmentMap() const;

		const Handles* getHandles() const;

		Material* getMaterial();

		CubeMap* getPrefilteredEnvironmentMap() const;

		static Texture2D* getBrdfLookupTexture();
		StoreImage readBackgroundPixelData() const;

		void init(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

	protected:

		class ProbeTechnique;
		class ProbeMaterial;

		void initBackground(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

		void initPrefiltered(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

		void initIrradiance(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);
		void loadIrradianceFile(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);
		//void createIrradianceTex(Texture* backgroundHDR, unsigned probeID, const std::filesystem::path& probeRoot);

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

		static std::shared_ptr<Texture2D> mBrdfLookupTexture;
		static std::unique_ptr<ProbeTechnique> mTechnique;
		static std::unique_ptr<SphereMesh> mMesh;
		static std::unique_ptr<Sampler> mSamplerIrradiance;
		static std::unique_ptr<Sampler> mSamplerPrefiltered;

		std::unique_ptr<ProbeMaterial> mMaterial;
		StoreImage mReadImage;
		Handles mHandles;
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