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
	class Probe;
	class GlobalIllumination;
	class ProbeManager;
	class ProbeFactory;


	class Probe : public Resource {

	public:

		class ProbePass;

		/**
		 * Specifies the influence volume type of a probe.
		 */
		enum class InfluenceType {
			SPHERE = 0,
			BOX = 1,
		};

		/**
		 * Specifies the type of the probe.
		 */
		enum class Type {
			Irradiance,
			Reflection
		};

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

		Probe(Type type, const glm::vec3& position, std::optional<Texture*> source, unsigned storeID);

		Probe(Probe&& o) noexcept = delete;
		Probe& operator=(Probe&& o) noexcept = delete;
		Probe(const Probe& o) noexcept = delete;
		Probe& operator=(const Probe& o) noexcept = delete;

		virtual ~Probe();

		/**
		 * Provides the array index of this probe.
		 */
		unsigned getArrayIndex() const;

		MeshGroup* getMeshGroup();

		const AABB& getInfluenceBox() const;
		float getInfluenceRadius() const;
		InfluenceType getInfluenceType() const;

		const glm::vec3& getPosition() const;

		const std::optional<Texture*>& getSource() const;

		unsigned getStoreID() const;

		Type getType() const;

		void init(unsigned storeID, unsigned arrayIndex, std::unique_ptr<ProbeMaterial> material, Mesh* mesh);

		bool isInitialized() const;

		void setInfluenceBox(const glm::vec3& halfWidth);
		void setInfluenceRadius(float radius);
		void setInfluenceType(InfluenceType type);

		void setPosition(const glm::vec3& position);

	protected:

		std::unique_ptr<ProbeMaterial> mMaterial;
		std::unique_ptr<MeshGroup> mMeshGroup;
		unsigned mStoreID;
		unsigned mArrayIndex;
		bool mInit;
		glm::vec3 mPosition;
		
		//Influence can be either defined by a sphere or an AABB
		InfluenceType mInfluenceType;
		float mInfluenceRadius;
		AABB mInfluenceBox;
		Type mType;

		/**
		 * An optional source texture used for initializing the probe
		 */
		std::optional<Texture*> mSource;
	};


	class ProbeFactory
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
		static constexpr char* STORE_FILE_BASE_SOURCE = "probe_source_";
		static constexpr char* STORE_FILE_BASE_IRRADIANCE = "probe_irradiance_";
		static constexpr char* STORE_FILE_BASE_IRRADIANCE_SH = "probe_irradiance_sh_";
		static constexpr char* STORE_FILE_BASE_REFLECTION = "probe_reflection_";
		static constexpr char* STORE_FILE_EXTENSION = ".probe";

		ProbeFactory(unsigned reflectionMapSize, unsigned irradianceArraySize, unsigned reflectionArraySize);

		ProbeFactory(const ProbeFactory&) = delete;
		ProbeFactory(ProbeFactory&&) = delete;

		ProbeFactory& operator=(const ProbeFactory&) = delete;
		ProbeFactory& operator=(ProbeFactory&&) = delete;

		static Texture2D* getBrdfLookupTexture();

		CubeMapArray* getIrradianceMaps();
		CubeMapArray* getReflectionMaps();
		const std::filesystem::path& getProbeRootDir() const;


		static void init(const std::filesystem::path& probeCompiledDirectory, std::string probeFileExtension);

		/**
		 * Non blocking init function for probes.
		 */
		void initProbe(ProbeVob& probeVob, const CubeMap* environmentMap, bool useCache, bool storeRenderedResult);

		/**
		 * Non blocking init function for probes.
		 */
		void initProbe(ProbeVob& probeVob, bool useCache, bool storeRenderedResult);


		bool isProbeStored(const Probe& probe) const;

		/**
		 * Loads an equirectangular texture to a cubemap, or if a stored cubemap with a given store id exists,
		 * the cubemap is loaded directly from file.
		 */
		std::shared_ptr<CubeMap> createCubeMap(const Texture* equirectangularTexture,
			unsigned storeID,
			bool useCache,
			bool storeRenderedResult);

		/**
		 * Renders an equirectangular texture to a cube map.
		 */
		static std::shared_ptr<CubeMap> renderEquirectangularToCube(const Texture* equirectangularTexture);

	private:

		static std::unique_ptr<MeshGroup> createSkyBox();
		static std::shared_ptr<Texture2D> createBRDFlookupTexture(Shader* brdfPrecompute);

		std::shared_ptr<CubeMap> convolute(const CubeMap* source);
		std::shared_ptr<CubeMap> createIrradianceSH(const Texture2D* shCoefficients);
		std::shared_ptr<CubeMap> prefilter(const CubeMap* source, unsigned prefilteredSize);
		void convoluteSphericalHarmonics(const CubeMap* source, Texture2D* output, unsigned rowIndex);


		void initReflection(const CubeMap* source,
			unsigned storeID,
			unsigned arrayIndex,
			const std::filesystem::path& probeRoot,
			unsigned reflectionMapSize,
			bool useCache,
			bool storeRenderedResult);

		void initIrradiance(const CubeMap* source,
			unsigned storeID,
			unsigned arrayIndex,
			const std::filesystem::path& probeRoot,
			bool useCache,
			bool storeRenderedResult);

		void initIrradianceSH(const Texture2D* shCoefficients,
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
		static std::shared_ptr<TypedOwningShaderProvider<Probe::ProbePass>> mProbeShaderProvider;
		static std::unique_ptr<SphereMesh> mMesh;
		static std::unique_ptr<Sampler> mSamplerIrradiance;
		static std::unique_ptr<Sampler> mSamplerPrefiltered;
		static std::unique_ptr<FileSystem> mFileSystem;

		std::unique_ptr<CubeMapArray> mIrradianceMaps;
		std::unique_ptr<CubeMapArray> mReflectionMaps;
		const unsigned mReflectionMapSize;

		const unsigned mIrradianceArraySize;
		unsigned mIrradianceFreeSlots;
		
		const unsigned mReflectionArraySize;
		unsigned mReflectionFreeSlots;

	};

	class ProbeVob : public Vob
	{
	public:

		virtual ~ProbeVob() = default;

		Probe* getProbe();

		void setPositionLocalToParent(const glm::vec3& position) override;

	protected:
		friend GlobalIllumination;
		friend ProbeManager;
		ProbeVob(Vob* parent, Probe* probe);
		

	private:
		Probe* mProbe;
	};
}