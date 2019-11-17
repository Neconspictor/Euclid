#pragma once

#include <nex/texture/Sprite.hpp>
#include <nex/texture/Image.hpp>
#include "PbrPass.hpp"
#include <nex/scene/Scene.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/resource/Resource.hpp>
#include <memory>
#include <nex/material/Material.hpp>

namespace nex
{
	class FileSystem;
	class SceneNode;
	class DirectionalLight;
	class MeshGroup;
	class SphereMesh;
	class Mesh;
	class CubeMapArray;
	class Sampler;
	class PbrProbe;
	class GlobalIllumination;

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
		const std::filesystem::path& getProbeRootDir() const;


		static void init(const std::filesystem::path& probeCompiledDirectory, std::string probeFileExtension);

		/**
		 * Non blocking init function for probes.
		 */
		void initProbeBackground(PbrProbe& probe, Texture* backgroundHDR, unsigned storeID, bool useCache, bool storeRenderedResult);

		/**
		 * Non blocking init function for probes.
		 */
		void initProbe(PbrProbe& probe, CubeMap* environmentMap, unsigned storeID, bool useCache, bool storeRenderedResult);

		/**
		 * Non blocking init function for probes.
		 */
		void initProbe(PbrProbe& probe, unsigned storeID, bool useCache, bool storeRenderedResult);
		
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

		class ProbePass;

		enum class InfluenceType {
			SPHERE = 0,
			BOX = 1,
		};

		class ProbeMaterial : public Material {
		public:

			ProbeMaterial(ProbePass* shader);

			void setProbeFactory(PbrProbeFactory* factory);

			void setArrayIndex(float index);

			PbrProbeFactory* mFactory;
			float mArrayIndex;
		};

		struct Handles {
			uint64_t convoluted = 0;
			uint64_t prefiltered = 0;
		};

		static constexpr unsigned IRRADIANCE_SIZE = 32;
		static constexpr unsigned BRDF_SIZE = 512;
		static const TextureDesc BRDF_DATA;
		static const TextureDesc IRRADIANCE_DATA;
		static const TextureDesc PREFILTERED_DATA;
		static const TextureDesc SOURCE_DATA;
		static constexpr unsigned SOURCE_CUBE_SIZE = 1024;
		static constexpr unsigned INVALID_STOREID = UINT32_MAX;
		static constexpr unsigned INVALID_ARRAY_INDEX = UINT32_MAX;

		PbrProbe(const glm::vec3& position, unsigned storeID = INVALID_STOREID);

		PbrProbe(PbrProbe&& o) noexcept = delete;
		PbrProbe& operator=(PbrProbe&& o) noexcept = delete;
		PbrProbe(const PbrProbe& o) noexcept = delete;
		PbrProbe& operator=(const PbrProbe& o) noexcept = delete;

		virtual ~PbrProbe();

		//void drawSky(const glm::mat4& projection,
		//	const glm::mat4& view);

		static void initGlobals(const std::filesystem::path& probeRoot);
		static Mesh* getSphere();

		/**
		 * Provides the array index of this probe.
		 */
		unsigned getArrayIndex() const;

		const AABB& getInfluenceBox() const;
		float getInfluenceRadius() const;
		InfluenceType getInfluenceType() const;


		CubeMapArray* getIrradianceMaps() const;

		const Handles* getHandles() const;

		Material* getMaterial();

		CubeMapArray* getPrefilteredMaps() const;

		static Texture2D* getBrdfLookupTexture();

		const glm::vec3& getPosition() const;

		unsigned getStoreID() const;

		void init(Texture* backgroundHDR, 
			unsigned prefilteredSize,
			unsigned storeID, 
			PbrProbeFactory* factory,
			unsigned arrayIndex,
			const std::filesystem::path& probeRoot, bool useCache,
			bool storeRenderedResult);

		void init(CubeMap* environment,
			unsigned prefilteredSize,
			unsigned storeID,
			PbrProbeFactory* factory,
			unsigned arrayIndex,
			const std::filesystem::path& probeRoot, bool useCache,
			bool storeRenderedResult);

		void init(unsigned prefilteredSize,
			unsigned storeID,
			PbrProbeFactory* factory,
			unsigned arrayIndex,
			const std::filesystem::path& probeRoot, bool useCache,
			bool storeRenderedResult);

		bool isInitialized() const;

		bool isSourceStored(const std::filesystem::path& probeRoot) const;

		void setInfluenceBox(const glm::vec3& halfWidth);
		void setInfluenceRadius(float radius);
		void setInfluenceType(InfluenceType type);

		void setPosition(const glm::vec3& position);

	protected:

		static constexpr char* STORE_FILE_EXTENSION = ".probe";

		std::shared_ptr<CubeMap> createSource(Texture* backgroundHDR, const std::filesystem::path& probeRoot, bool useCache, bool storeRenderedResult);

		void initPrefiltered(CubeMap* source, unsigned prefilteredSize, const std::filesystem::path& probeRoot, bool useCache, bool storeRenderedResult);

		void initIrradiance(CubeMap* source, const std::filesystem::path& probeRoot, bool useCache, bool storeRenderedResult);
		void initIrradianceSH(Texture2D* shCoefficients, const std::filesystem::path& probeRoot, bool useCache, bool storeRenderedResult);

		static std::unique_ptr<MeshGroup> createSkyBox();
		static std::shared_ptr<Texture2D> createBRDFlookupTexture(Shader* brdfPrecompute);

		static StoreImage loadCubeMap(const std::filesystem::path& probeRoot,
			const std::string& baseName,
			unsigned storeID,
			bool useCache,
			bool storeRenderedResult,
			const std::function<std::shared_ptr<CubeMap>()>& renderFunc);

		std::shared_ptr<CubeMap> renderBackgroundToCube(Texture* background);
		std::shared_ptr<CubeMap> convolute(CubeMap* source);
		std::shared_ptr<CubeMap> createIrradianceSH(Texture2D* shCoefficients);
		std::shared_ptr<CubeMap> prefilter(CubeMap* source, unsigned prefilteredSize);
		void convoluteSphericalHarmonics(CubeMap* source, Texture2D* output, unsigned rowIndex);

		static std::shared_ptr<Texture2D> mBrdfLookupTexture;
		static std::unique_ptr<ProbePass> mProbePass;
		static std::unique_ptr<SphereMesh> mMesh;
		static std::unique_ptr<Sampler> mSamplerIrradiance;
		static std::unique_ptr<Sampler> mSamplerPrefiltered;

		std::unique_ptr<ProbeMaterial> mMaterial;
		Handles mHandles;
		unsigned mStoreID;
		PbrProbeFactory* mFactory;
		unsigned mArrayIndex;
		bool mInit;
		glm::vec3 mPosition;
		
		//Influence can be either defined by a sphere or an AABB
		InfluenceType mInfluenceType;
		float mInfluenceRadius;
		AABB mInfluenceBox;
	};

	class ProbeVob : public Vob
	{
	public:
		virtual ~ProbeVob() = default;

		PbrProbe* getProbe();

		void setPosition(const glm::vec3& position) override;

	protected:
		friend GlobalIllumination;
		ProbeVob(SceneNode* meshRootNode, PbrProbe* probe);
		

	private:
		PbrProbe* mProbe;
	};
}