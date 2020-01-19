#pragma once

#include <nex/texture/Sprite.hpp>
#include <nex/texture/Image.hpp>
#include <nex/pbr/PbrPass.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/resource/Resource.hpp>
#include <memory>
#include <nex/material/Material.hpp>
#include <nex/shader/ShaderProvider.hpp>

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
	class ProbeManager;
	class PbrProbeFactory;


	class PbrProbe : public Resource {

	public:

		class ProbePass;

		enum class InfluenceType {
			SPHERE = 0,
			BOX = 1,
		};

		static constexpr unsigned INVALID_STOREID = UINT32_MAX;

		using ProbeShaderProvider = std::shared_ptr<TypedShaderProvider<ProbePass>>;

		class ProbeMaterial : public Material {
		public:

			ProbeMaterial(ProbeShaderProvider provider);

			void setIrradianceMaps(CubeMapArray* irradianceMaps);
			void setReflectionMaps(CubeMapArray* reflectionMaps);

			void setArrayIndex(float index);

			CubeMapArray* mIrradianceMaps;
			CubeMapArray* mReflectionMaps;
			float mArrayIndex;
		};

		struct Handles {
			uint64_t irradiance = 0;
			uint64_t reflection = 0;
		};

		PbrProbe(const glm::vec3& position, unsigned storeID = INVALID_STOREID);

		PbrProbe(PbrProbe&& o) noexcept = delete;
		PbrProbe& operator=(PbrProbe&& o) noexcept = delete;
		PbrProbe(const PbrProbe& o) noexcept = delete;
		PbrProbe& operator=(const PbrProbe& o) noexcept = delete;

		virtual ~PbrProbe();

		/**
		 * Provides the array index of this probe.
		 */
		unsigned getArrayIndex() const;

		MeshGroup* getMeshGroup();

		const AABB& getInfluenceBox() const;
		float getInfluenceRadius() const;
		InfluenceType getInfluenceType() const;

		const Handles* getHandles() const;

		const glm::vec3& getPosition() const;

		unsigned getStoreID() const;

		void init(unsigned storeID, unsigned arrayIndex, std::unique_ptr<ProbeMaterial> material, Mesh* mesh);

		bool isInitialized() const;

		bool isSourceStored(const std::filesystem::path& probeRoot) const;

		void setInfluenceBox(const glm::vec3& halfWidth);
		void setInfluenceRadius(float radius);
		void setInfluenceType(InfluenceType type);

		void setPosition(const glm::vec3& position);

	protected:

		std::unique_ptr<ProbeMaterial> mMaterial;
		std::unique_ptr<MeshGroup> mMeshGroup;
		Handles mHandles;
		unsigned mStoreID;
		unsigned mArrayIndex;
		bool mInit;
		glm::vec3 mPosition;
		
		//Influence can be either defined by a sphere or an AABB
		InfluenceType mInfluenceType;
		float mInfluenceRadius;
		AABB mInfluenceBox;
	};


	class PbrProbeFactory
	{
	public:

		static constexpr unsigned IRRADIANCE_SIZE = 32;
		static constexpr unsigned BRDF_SIZE = 512;
		static const TextureDesc BRDF_DATA;
		static const TextureDesc IRRADIANCE_DATA;
		static const TextureDesc REFLECTION_DATA;
		static const TextureDesc SOURCE_DATA;
		static constexpr unsigned SOURCE_CUBE_SIZE = 1024;
		static constexpr unsigned INVALID_STOREID = UINT32_MAX;
		static constexpr unsigned INVALID_ARRAY_INDEX = UINT32_MAX;
		static constexpr char* STORE_FILE_EXTENSION = ".probe";

		PbrProbeFactory(unsigned reflectionMapSize, unsigned probeArraySize);

		PbrProbeFactory(const PbrProbeFactory&) = delete;
		PbrProbeFactory(PbrProbeFactory&&) = delete;

		PbrProbeFactory& operator=(const PbrProbeFactory&) = delete;
		PbrProbeFactory& operator=(PbrProbeFactory&&) = delete;

		static Texture2D* getBrdfLookupTexture();

		CubeMapArray* getIrradianceMaps();
		CubeMapArray* getReflectionMaps();
		const std::filesystem::path& getProbeRootDir() const;


		static void init(const std::filesystem::path& probeCompiledDirectory, std::string probeFileExtension);

		/**
		 * Non blocking init function for probes.
		 */
		void initProbeBackground(ProbeVob& probeVob, Texture* backgroundHDR, unsigned storeID, bool useCache, bool storeRenderedResult);

		/**
		 * Non blocking init function for probes.
		 */
		void initProbe(ProbeVob& probeVob, CubeMap* environmentMap, unsigned storeID, bool useCache, bool storeRenderedResult);

		/**
		 * Non blocking init function for probes.
		 */
		void initProbe(ProbeVob& probeVob, unsigned storeID, bool useCache, bool storeRenderedResult);

	private:

		static std::unique_ptr<MeshGroup> createSkyBox();
		static std::shared_ptr<Texture2D> createBRDFlookupTexture(Shader* brdfPrecompute);

		std::shared_ptr<CubeMap> createSource(Texture* backgroundHDR, 
			unsigned storeID,
			const std::filesystem::path& probeRoot, 
			bool useCache, 
			bool storeRenderedResult);

		std::shared_ptr<CubeMap> renderBackgroundToCube(Texture* background);
		std::shared_ptr<CubeMap> convolute(CubeMap* source);
		std::shared_ptr<CubeMap> createIrradianceSH(Texture2D* shCoefficients);
		std::shared_ptr<CubeMap> prefilter(CubeMap* source, unsigned prefilteredSize);
		void convoluteSphericalHarmonics(CubeMap* source, Texture2D* output, unsigned rowIndex);


		void initReflection(CubeMap* source,
			unsigned storeID,
			unsigned arrayIndex,
			const std::filesystem::path& probeRoot,
			unsigned reflectionMapSize,
			bool useCache,
			bool storeRenderedResult);

		void initIrradiance(CubeMap* source,
			unsigned storeID,
			unsigned arrayIndex,
			const std::filesystem::path& probeRoot,
			bool useCache,
			bool storeRenderedResult);

		void initIrradianceSH(Texture2D* shCoefficients,
			unsigned storeID,
			unsigned arrayIndex,
			const std::filesystem::path& probeRoot,
			bool useCache,
			bool storeRenderedResult);

		static StoreImage loadCubeMap(const std::filesystem::path& probeRoot,
			const std::string& baseName,
			unsigned storeID,
			bool useCache,
			bool storeRenderedResult,
			const std::function<std::shared_ptr<CubeMap>()>& renderFunc);

		static std::shared_ptr<Texture2D> mBrdfLookupTexture;
		static std::shared_ptr<TypedOwningShaderProvider<PbrProbe::ProbePass>> mProbeShaderProvider;
		static std::unique_ptr<SphereMesh> mMesh;
		static std::unique_ptr<Sampler> mSamplerIrradiance;
		static std::unique_ptr<Sampler> mSamplerPrefiltered;
		static std::unique_ptr<FileSystem> mFileSystem;

		std::unique_ptr<CubeMapArray> mIrradianceMaps;
		std::unique_ptr<CubeMapArray> mReflectionMaps;
		const unsigned mReflectionMapSize;
		const unsigned mProbeArraySize;
		unsigned mFreeSlots;

	};

	class ProbeVob : public Vob
	{
	public:

		virtual ~ProbeVob() = default;

		PbrProbe* getProbe();

		void setPositionLocalToParent(const glm::vec3& position) override;

	protected:
		friend GlobalIllumination;
		friend ProbeManager;
		ProbeVob(Vob* parent, PbrProbe* probe);
		

	private:
		PbrProbe* mProbe;
	};
}