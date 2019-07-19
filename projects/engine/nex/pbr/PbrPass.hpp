#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/shader/ShaderBuffer.hpp>
#include <nex/texture/Sampler.hpp>
#include "nex/light/Light.hpp"


namespace nex
{
	class GlobalIllumination;

	class PbrBaseCommon
	{
	public:
		PbrBaseCommon(Shader* shader);

		virtual ~PbrBaseCommon() = default;

		void setShader(Shader* shader);

		virtual void updateConstants(const Camera& camera) = 0;

	protected:
		Shader* mShader;
	};

	class PbrGeometryData : public PbrBaseCommon
	{
	public:

		static const unsigned ALBEDO_BINDING_POINT = 0;
		static const unsigned AO_BINDING_POINT = 1;
		static const unsigned METALLIC_BINDING_POINT = 2;
		static const unsigned NORMAL_BINDING_POINT = 3;
		static const unsigned ROUGHNESS_BINDING_POINT = 4;

		PbrGeometryData(Shader* shader);


		void setAlbedoMap(const Texture* albedo);
		void setAoMap(const Texture* ao);
		void setMetalMap(const Texture* metal);
		void setNormalMap(const Texture* normal);
		void setRoughnessMap(const Texture* roughness);

		void updateConstants(const Camera& camera);

	private:

		void setNearFarPlane(const glm::vec2& nearFarPlane);

		// mesh material
		UniformTex mAlbedoMap;
		UniformTex mAmbientOcclusionMap;
		UniformTex mMetalMap;
		UniformTex mNormalMap;
		UniformTex mRoughnessMap;

		Uniform mNearFarPlane;

		Sampler* mDefaultImageSampler;
	};

	class PbrLightingData : public PbrBaseCommon
	{
	public:

		PbrLightingData(Shader* shader, GlobalIllumination* globalIllumination, 
			CascadedShadow* cascadedShadow, unsigned csmCascadeBufferBindingPoint = 0, unsigned pbrProbesBufferBindingPoint = 1);

		void setCascadedShadow(CascadedShadow* shadow);

		/**
		 * Updates constants (constant properties for all submesh drawings)
		 */
		void updateConstants(const Camera& camera);

		void updateLight(const DirectionalLight & light, const Camera& camera);

	private:
		void setArrayIndex(float index);
		void setBrdfLookupTexture(const Texture* brdfLUT);
		void setIrradianceMaps(const CubeMapArray* texture);
		void setPrefilteredMaps(const CubeMapArray* texture);

		void setEyeLightDirection(const glm::vec3& direction);
		void setLightColor(const glm::vec3& color);
		void setLightPower(float power);
		void setAmbientLightPower(float power);

		void setInverseViewMatrix(const glm::mat4& mat);

		void setCascadedDepthMap(const Texture* cascadedDepthMap);
		//void setCascadedData(const CascadedShadow::CascadeData& cascadedData);
		void setShadowStrength(float strength);

		void setNearFarPlane(const glm::vec2& nearFarPlane);


		//ibl
		UniformTex mBrdfLUT;
		UniformTex mIrradianceMaps;
		UniformTex mPrefilteredMaps;
		Uniform mArrayIndex;


		// CSM
		UniformTex mCascadedDepthMap;
		//ShaderStorageBuffer cascadeBufferUBO; //UniformBuffer ShaderStorageBuffer


		Uniform mEyeLightDirection;
		Uniform mLightColor;
		Uniform mLightPower;
		Uniform mAmbientLightPower;
		Uniform mShadowStrength;

		Uniform mInverseView;

		Uniform mNearFarPlane;
		Sampler mSampler;
		Sampler mPrefilteredSampler;
		Sampler mCascadedShadowMapSampler;

		unsigned mPbrProbesDataBufferBindingPoint;
		GlobalIllumination* mGlobalIllumination;

		unsigned mCsmCascadeBindingPoint;
		CascadedShadow* mCascadeShadow;
	};

	class PbrGeometryPass : public TransformPass {
	public:
		PbrGeometryPass(std::unique_ptr<Shader> program = nullptr, unsigned transformBindingPoint = 0);
		PbrGeometryData* getShaderInterface();

	protected:
		PbrGeometryData mGeometryData;
	};



	class PbrForwardPass : public PbrGeometryPass
	{
	public:
		PbrForwardPass(const ShaderFilePath& vertexShader, const ShaderFilePath& fragmentShader, 
			GlobalIllumination* globalIllumination, CascadedShadow* cascadedShadow);

		void updateConstants(const Camera& camera) override;

		void updateLight(const DirectionalLight& light, const Camera& camera);

	private:
		PbrLightingData mLightingPass;

		static constexpr unsigned CASCADE_BUFFER_BINDINGPOINT = 0;
		static constexpr unsigned TRANSFORM_BUFFER_BINDINGPOINT = 1;
		static constexpr unsigned PBR_PROBES_BUFFER_BINDINPOINT = 2;

		static std::vector<std::string> generateDefines(CascadedShadow* cascadedShadow);
	};

	class PbrDeferredGeometryPass : public PbrGeometryPass {
	public:
		PbrDeferredGeometryPass(std::unique_ptr<Shader> shader);

		void updateConstants(const Camera& camera) override;
	};

	class PbrDeferredLightingPass : public Pass {
	public:

		PbrDeferredLightingPass(const ShaderFilePath& vertexShader, const ShaderFilePath& fragmentShader, 
			GlobalIllumination* globalIllumination, CascadedShadow* cascadedShadow);

		void setAlbedoMap(const Texture* texture);
		void setAoMetalRoughnessMap(const Texture* texture);
		void setNormalEyeMap(const Texture* texture);
		void setNormalizedViewSpaceZMap(const Texture* texture);

		void setInverseProjMatrixFromGPass(const glm::mat4& mat);

		void updateConstants(const Camera& camera) override;
		void updateLight(const DirectionalLight& light, const Camera& camera);

	private:
		UniformTex mAlbedoMap;
		UniformTex mAoMetalRoughnessMap;
		UniformTex mNormalEyeMap;
		UniformTex mNormalizedViewSpaceZMap;

		Uniform mInverseProjFromGPass;

		PbrLightingData mLightingPass;

		static constexpr unsigned CASCADE_BUFFER_BINDINGPOINT = 0;
		static constexpr unsigned PBR_PROBES_BUFFER_BINDINPOINT = 1;

		static std::vector<std::string> generateDefines(CascadedShadow* cascadedShadow);
	};

	class PbrConvolutionPass : public Pass
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
	};

	class PbrPrefilterPass : public Pass
	{
	public:
		PbrPrefilterPass();

		virtual ~PbrPrefilterPass() = default;

		void setMapToPrefilter(CubeMap* cubeMap);

		void setRoughness(float roughness);

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);

	private:
		Uniform mProjection;
		Uniform mView;
		UniformTex mEnvironmentMap;
		Uniform mRoughness;
	};

	class PbrBrdfPrecomputePass : public Pass
	{
	public:
		PbrBrdfPrecomputePass();

		virtual ~PbrBrdfPrecomputePass() = default;
	};
}