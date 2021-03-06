#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/texture/Sampler.hpp>
#include "nex/light/Light.hpp"
#include <nex/material/Material.hpp>
#include "interface/buffers.h"
#include <vector>


namespace nex
{
	class GlobalIllumination;

	struct PbrConstants {
		glm::uvec4 windowDimension;
		glm::uvec4 clusterDimension;
		glm::vec4 nearFarDistance;
	};

	class PbrBaseCommon
	{
	public:
		PbrBaseCommon(ShaderProgram* shader);

		virtual ~PbrBaseCommon() = default;

		void setProgram(ShaderProgram* shader);

		virtual void updateConstants(const RenderContext& constants) = 0;

	protected:
		ShaderProgram* mShader;
	};

	class PbrGeometryData : public PbrBaseCommon
	{
	public:

		static const unsigned ALBEDO_BINDING_POINT = 0;
		static const unsigned AO_BINDING_POINT = 1;
		static const unsigned METALLIC_BINDING_POINT = 2;
		static const unsigned NORMAL_BINDING_POINT = 3;
		static const unsigned ROUGHNESS_BINDING_POINT = 4;
		static const unsigned EMISSION_BINDING_POINT = 5;

		PbrGeometryData(ShaderProgram* shader);
		virtual ~PbrGeometryData() = default;


		void setAlbedoMap(const Texture* albedo);
		void setAoMap(const Texture* ao);
		void setMetalMap(const Texture* metal);
		void setNormalMap(const Texture* normal);
		void setRoughnessMap(const Texture* roughness);
		void setEmissionMap(const Texture* emission);

		void updateConstants(const RenderContext& constants) override;

	private:

		void setNearFarPlane(const glm::vec2& nearFarPlane);

		// mesh material
		UniformTex mAlbedoMap;
		UniformTex mAmbientOcclusionMap;
		UniformTex mMetalMap;
		UniformTex mNormalMap;
		UniformTex mRoughnessMap;
		UniformTex mEmissionMap;

		Uniform mNearFarPlane;
	};

	class PbrGeometryBonesData : public PbrGeometryData
	{
	public:
		PbrGeometryBonesData(ShaderProgram* shader, unsigned bonesBufferBindingPoint = 1);
		virtual ~PbrGeometryBonesData() = default;

		/**
		 * Binds the bone transformations.
		 * @param buffer : Is expected to have an glm::mat4 array representing bone transformations
		 */
		void bindBonesBuffer(ShaderStorageBuffer* buffer);

	private:
		unsigned mBonesBufferBindingPoint;
	};

	class PbrLightingData : public PbrBaseCommon
	{
	public:

		PbrLightingData(ShaderProgram* shader, GlobalIllumination* globalIllumination, 
			CascadedShadow* cascadedShadow, unsigned envLightBindingPoint = 1,
			unsigned envLightGlobalLightIndicesBindingPoint = 2,
			unsigned envLightLightGridsBindingPoint = 3,
			unsigned clustersAABBBindingPoint = 4);

		void setCascadedShadow(CascadedShadow* shadow);

		/**
		 * Updates constants (constant properties for all submesh drawings)
		 */
		void updateConstants(const RenderContext& constants);

	private:

		void setBrdfLookupTexture(const Texture* brdfLUT);
		void setIrradianceMaps(const Texture1DArray* texture);
		void setReflectionMaps(const CubeMapArray* texture);

		void setCascadedDepthMap(const Texture* cascadedDepthMap);
		void setNearFarPlane(const glm::vec2& nearFarPlane);


		//ibl
		UniformTex mBrdfLUT;
		UniformTex mIrradianceMaps;
		UniformTex mReflectionMaps;

		// CSM
		UniformTex mCascadedDepthMap;
		//ShaderStorageBuffer cascadeBufferUBO; //UniformBuffer ShaderStorageBuffer

		Uniform mNearFarPlane;
		Sampler mSampler;
		Sampler mReflectionSampler;
		Sampler mCascadedShadowMapSampler;

		unsigned mEnvLightBindingPoint;
		GlobalIllumination* mGlobalIllumination;

		CascadedShadow* mCascadeShadow;

		unsigned mEnvLightGlobalLightIndicesBindingPoint;
		unsigned mEnvLightLightGridsBindingPoint;
		unsigned mClustersAABBBindingPoint;
	};


	class BasePbrGeometryShader : public TransformShader {
	public:
		BasePbrGeometryShader(std::unique_ptr<ShaderProgram> program = nullptr);
		virtual ~BasePbrGeometryShader() = default;
	};

	class PbrGeometryShader : public BasePbrGeometryShader {
	public:
		PbrGeometryShader(std::unique_ptr<ShaderProgram> program = nullptr);
		virtual ~PbrGeometryShader() = default;
		PbrGeometryData* getShaderInterface();

		void updateMaterial(const Material& material) override;

	protected:
		PbrGeometryData mGeometryData;
	};

	class PbrGeometryBonesShader : public BasePbrGeometryShader {
	public:
		PbrGeometryBonesShader(std::unique_ptr<ShaderProgram> program = nullptr, unsigned bonesBufferBindinPoint = 1);
		virtual ~PbrGeometryBonesShader() = default;
		PbrGeometryBonesData* getShaderInterface();

		void updateMaterial(const Material& material) override;

	protected:
		PbrGeometryBonesData mGeometryBonesData;
	};



	class PbrForwardPass : public PbrGeometryShader
	{
	public:
		PbrForwardPass(const ShaderFilePath& vertexShader, const ShaderFilePath& fragmentShader, 
			GlobalIllumination* globalIllumination, CascadedShadow* cascadedShadow, const std::vector<std::string>& defines);

		void updateConstants(const RenderContext& constants) override;

	private:
		PbrLightingData mLightingPass;

		static constexpr unsigned PBR_PROBES_BUFFER_BINDINPOINT = 2;

		static std::vector<std::string> generateDefines(const std::vector<std::string>& defines, CascadedShadow* cascadedShadow);
	};

	class PbrDeferredGeometryShader : public PbrGeometryShader {
	public:
		PbrDeferredGeometryShader(std::unique_ptr<ShaderProgram> shader);
		virtual ~PbrDeferredGeometryShader() = default;

		void updateConstants(const RenderContext& constants) override;
	};

	class PbrDeferredGeometryBonesShader : public PbrGeometryBonesShader {
	public:
		PbrDeferredGeometryBonesShader(std::unique_ptr<ShaderProgram> shader);
		virtual ~PbrDeferredGeometryBonesShader() = default;

		void updateConstants(const RenderContext& constants) override;
	};

	class PbrDeferredAmbientPass : public Shader {
	public:

		PbrDeferredAmbientPass(GlobalIllumination* globalIllumination, bool useConeTracing);

		void setAlbedoMap(const Texture* texture);
		void setAoMetalRoughnessMap(const Texture* texture);
		void setNormalEyeMap(const Texture* texture);
		void setDepthMap(const Texture* texture);
		void setEmissionObjectMaterialIDMap(const Texture* texture);

		void setBrdfLookupTexture(const Texture* texture);
		void setIrradianceMaps(const Texture* texture);
		void setPrefilteredMaps(const Texture* texture);
		void setIrradianceArrayIndex(float index);
		void setReflectionArrayIndex(float index);

		void setAmbientLightPower(float power);

		void updateConstants(const RenderContext& constants) override;

	private:
		UniformTex mAlbedoMap;
		UniformTex mAoMetalRoughnessMap;
		UniformTex mNormalEyeMap;
		UniformTex mDepthMap;
		UniformTex mEmissionObjectMaterialIDMap;

		UniformTex mIrradianceMaps;
		UniformTex mPrefilteredMaps;
		UniformTex mBrdfLUT;
		Uniform mIrradianceArrayIndex;
		Uniform mReflectionArrayIndex;

		UniformTex mVoxelTexture;
		
		
		Uniform mAmbientLightPower;
		GlobalIllumination* mGlobalIllumination;
		bool mUseConeTracing;
		Sampler mVoxelSampler;

		// textures
		static constexpr unsigned PBR_ALBEDO_BINDINPOINT = 0;
		static constexpr unsigned PBR_AO_METAL_ROUGHNESS_BINDINPOINT = 1;
		static constexpr unsigned PBR_NORMAL_BINDINPOINT = 2;
		static constexpr unsigned PBR_DEPTH_BINDINPOINT = 3;
		static constexpr unsigned PBR_EMISSION_OBJECT_MATERIAL_ID_BINDINPOINT = 4; 
		static constexpr unsigned PBR_IRRADIANCE_BINDING_POINT = 6;
		static constexpr unsigned PBR_PREFILTERED_BINDING_POINT = 7;
		static constexpr unsigned PBR_BRDF_LUT_BINDING_POINT = 8;
		static constexpr unsigned VOXEL_TEXTURE_BINDING_POINT = 10;

		//shader storage buffers
		static constexpr unsigned PBR_PROBES_BUFFER_BINDINPOINT = 1;
		static constexpr unsigned PBR_ENVIRONMENT_LIGHTS_GLOBAL_LIGHT_INDICES = 2;
		static constexpr unsigned PBR_ENVIRONMENT_LIGHTS_LIGHT_GRIDS = 3;
		static constexpr unsigned PBR_CLUSTERS_AABB = 4;

		static std::vector<std::string> generateDefines(bool useConeTracing);
	};

	class PbrDeferredLightingPass : public Shader {
	public:

		PbrDeferredLightingPass(const ShaderFilePath& vertexShader, const ShaderFilePath& fragmentShader, 
			GlobalIllumination* globalIllumination, CascadedShadow* cascadedShadow);

		void setAlbedoMap(const Texture* texture);
		void setAoMetalRoughnessMap(const Texture* texture);
		void setNormalEyeMap(const Texture* texture);
		void setDepthMap(const Texture* texture);
		void setEmissionObjectMaterialIDMap(const Texture* texture);

		void setIrradianceOutMap(const Texture* texture);
		void setAmbientReflectionOutMap(const Texture* texture);

		void updateConstants(const RenderContext& constants) override;

	private:
		UniformTex mAlbedoMap;
		UniformTex mAoMetalRoughnessMap;
		UniformTex mNormalEyeMap;
		UniformTex mDepthMap;
		UniformTex mEmissionObjectMaterialIDMap;
		UniformTex mIrradianceOutMap;
		UniformTex mAmbientReflectionOutMap;


		PbrLightingData mLightingPass;

		static constexpr unsigned PBR_PROBES_BUFFER_BINDINPOINT = 1;

		static constexpr unsigned PBR_IRRADIANCE_OUT_MAP_BINDINGPOINT = 11;
		static constexpr unsigned PBR_AMBIENT_REFLECTION_OUT_MAP_BINDINGPOINT = 12;

		static std::vector<std::string> generateDefines(CascadedShadow* cascadedShadow);
	};

	class PbrConvolutionPass : public Shader
	{
	public:
		PbrConvolutionPass();

		virtual ~PbrConvolutionPass() = default;

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);
		void setEnvironmentMap(const CubeMap* cubeMap);

	private:
		Uniform mProjection;
		Uniform mView;
		UniformTex mEnvironmentMap;
		Sampler mSampler;
		Sampler mSampler2;
	};

	class PbrIrradianceShPass : public Shader
	{
	public:
		PbrIrradianceShPass();

		virtual ~PbrIrradianceShPass() = default;

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);
		void setCoefficientMap(const Texture1DArray* coefficients);
		void setArrayIndex(int arrayIndex);

	private:
		Uniform mProjection;
		Uniform mView;
		UniformTex mCoefficientMap;
		Uniform mArrayIndex;
		Sampler mSampler;
	};


	/**
	 * Computes Spherical harmonics of an environment map
	 */
	class SHComputePass : public ComputeShader
	{
	public:
		SHComputePass();

		virtual ~SHComputePass() = default;

		/**
		 * @param texture: Sets the output texture where the SH coefficients will be written to.
		 * Note: The texture has to have internal format InternFormat::RGB32F
		 * @param mipmap : The mipmap level of the texture to write to.
		 * @param environmentMaps: An array of environment maps used for generating the sh coefficients.
		 * @param rowStart : The starting index used for locating the first environment map and the first row in the output texture.
		 * @param rowCount : Specifies how much sets of sh coefficients should be generated.
		 */
		void compute(Texture1DArray* texture, unsigned mipmap, const CubeMap* environmentMaps, unsigned rowStart, unsigned rowCount);

	private:
		Uniform mRowStart;
		UniformTex mEnvironmentMap;
		UniformTex mOutputMap;
		Sampler mSampler;
	};


	class PbrPrefilterPass : public Shader
	{
	public:
		PbrPrefilterPass();

		virtual ~PbrPrefilterPass() = default;

		void setMapToPrefilter(const CubeMap* cubeMap);

		void setRoughness(float roughness);

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);

	private:
		Uniform mProjection;
		Uniform mView;
		UniformTex mEnvironmentMap;
		Uniform mRoughness;
	};

	class PbrBrdfPrecomputePass : public Shader
	{
	public:
		PbrBrdfPrecomputePass();

		virtual ~PbrBrdfPrecomputePass() = default;
	};
}