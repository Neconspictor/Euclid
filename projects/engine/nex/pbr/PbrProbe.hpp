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
	class PbrProbe;

	class PbrProbeFactory
	{
	public:

		PbrProbeFactory(unsigned prefilteredSize, unsigned mapSize);

		PbrProbeFactory(const PbrProbeFactory&) = delete;
		PbrProbeFactory(PbrProbeFactory&&) = delete;

		PbrProbeFactory& operator=(const PbrProbeFactory&) = delete;
		PbrProbeFactory& operator=(PbrProbeFactory&&) = delete;

		CubeMapArray* getIrradianceMaps();
		CubeMapArray* getPrefilteredMaps();


		static void init(const std::filesystem::path& probeCompiledDirectory, std::string probeFileExtension);

		/**
		 * Non blocking init function for probes.
		 */
		void initProbe(PbrProbe* probe, Texture* backgroundHDR, unsigned storeID);
		
	private:
		static std::unique_ptr<FileSystem> mFileSystem;
		std::unique_ptr<CubeMapArray> mIrradianceMaps;
		std::unique_ptr<CubeMapArray> mPrefilteredMaps;
		const unsigned mPrefilteredSide; 
		const unsigned mMapSize;
		unsigned mFreeSlots;
		
	};

	class PbrProbe : public Resource {

	public:

		struct Handles {
			uint64_t convoluted = 0;
			uint64_t prefiltered = 0;
		};

		static constexpr unsigned IRRADIANCE_SIZE = 32;
		static constexpr unsigned BRDF_SIZE = 512;
		static const TextureData BRDF_DATA;
		static const TextureData IRRADIANCE_DATA;
		static const TextureData PREFILTERED_DATA;
		static const TextureData SOURCE_DATA;
		static constexpr unsigned SOURCE_CUBE_SIZE = 1024;

		PbrProbe();

		PbrProbe(PbrProbe&& o) noexcept = delete;
		PbrProbe& operator=(PbrProbe&& o) noexcept = delete;
		PbrProbe(const PbrProbe& o) noexcept = delete;
		PbrProbe& operator=(const PbrProbe& o) noexcept = delete;

		virtual ~PbrProbe();

		//void drawSky(const glm::mat4& projection,
		//	const glm::mat4& view);

		static void initGlobals(const std::filesystem::path& probeRoot);
		static Mesh* getSphere();

		unsigned getArrayIndex() const;

		CubeMapArray* getIrradianceMaps() const;

		const Handles* getHandles() const;

		Material* getMaterial();

		CubeMapArray* getPrefilteredMaps() const;

		static Texture2D* getBrdfLookupTexture();

		void init(Texture* backgroundHDR, 
			unsigned prefilteredSize,
			unsigned storeID, 
			PbrProbeFactory* factory,
			unsigned arrayIndex,
			const std::filesystem::path& probeRoot);

	protected:

		class ProbeTechnique;
		class ProbeMaterial;

		std::shared_ptr<CubeMap> createSource(Texture* backgroundHDR, const std::filesystem::path& probeRoot);

		void initPrefiltered(CubeMap* source, unsigned prefilteredSize, const std::filesystem::path& probeRoot);

		void initIrradiance(CubeMap* source, const std::filesystem::path& probeRoot);

		static std::unique_ptr<StaticMeshContainer> createSkyBox();
		static std::shared_ptr<Texture2D> createBRDFlookupTexture(Pass* brdfPrecompute);

		std::shared_ptr<CubeMap> renderBackgroundToCube(Texture* background);
		std::shared_ptr<CubeMap> convolute(CubeMap* source);
		std::shared_ptr<CubeMap> prefilter(CubeMap* source, unsigned prefilteredSize);

		static std::shared_ptr<Texture2D> mBrdfLookupTexture;
		static std::unique_ptr<ProbeTechnique> mTechnique;
		static std::unique_ptr<SphereMesh> mMesh;
		static std::unique_ptr<Sampler> mSamplerIrradiance;
		static std::unique_ptr<Sampler> mSamplerPrefiltered;

		std::unique_ptr<ProbeMaterial> mMaterial;
		Handles mHandles;
		unsigned mStoreID;
		PbrProbeFactory* mFactory;
		unsigned mArrayIndex;
	};

	class ProbeVob : public Vob
	{
	public:

		struct ProbeData {
			glm::vec4 arrayIndex;  // only first component is used
			glm::vec4 positionWorld; // last component isn't used
		};

		ProbeVob(SceneNode* meshRootNode, PbrProbe* probe);
		virtual ~ProbeVob() = default;

		PbrProbe* getProbe();

		const ProbeData* getProbeData() const;

		void updateProbeData();

	private:
		PbrProbe* mProbe;
		ProbeData mData;
	};
}