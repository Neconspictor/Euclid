#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/shader/ShaderBuffer.hpp>
#include <nex/texture/Sampler.hpp>
#include "nex/light/Light.hpp"


namespace nex
{
	class PbrProbe;

	class PbrBaseCommon
	{
	public:
		PbrBaseCommon(Shader* shader);

		virtual ~PbrBaseCommon() = default;

		void setShader(Shader* shader);

		virtual void updateConstants(Camera* camera) = 0;

	protected:
		Shader* mShader;
	};

	class PbrCommonGeometryPass : public PbrBaseCommon
	{
	public:

		static const unsigned ALBEDO_BINDING_POINT = 0;
		static const unsigned AO_BINDING_POINT = 1;
		static const unsigned METALLIC_BINDING_POINT = 2;
		static const unsigned NORMAL_BINDING_POINT = 3;
		static const unsigned ROUGHNESS_BINDING_POINT = 4;

		PbrCommonGeometryPass(Shader* shader);

		void updateConstants(Camera* camera);

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

	class PbrCommonLightingPass : public PbrBaseCommon
	{
	public:
		PbrCommonLightingPass(Shader* shader, CascadedShadow* cascadedShadow, unsigned csmCascadeBufferBindingPoint = 0);

		void setAmbientLight(AmbientLight* light);
		void setCascadedShadow(CascadedShadow* shadow);
		void setDirLight(DirectionalLight* light);
		void setProbe(PbrProbe* probe);

		/**
		 * Updates constants (constant properties for all submesh drawings)
		 */
		void updateConstants(Camera* camera);

	private:

		void setBrdfLookupTexture(const Texture* brdfLUT);
		void setIrradianceMap(const CubeMap* irradianceMap);
		void setPrefilterMap(const CubeMap* prefilterMap);

		void setEyeLightDirection(const glm::vec3& direction);
		void setLightColor(const glm::vec3& color);
		void setLightPower(float power);
		void setAmbientLightPower(float power);

		void setInverseViewMatrix(const glm::mat4& mat);

		void setCascadedDepthMap(const Texture* cascadedDepthMap);
		//void setCascadedData(const CascadedShadow::CascadeData& cascadedData);
		void setShadowStrength(float strength);

		void setNearFarPlane(const glm::vec2& nearFarPlane);

		unsigned mCsmCascadeBindingPoint;

		//ibl
		UniformTex mBrdfLUT;
		UniformTex mIrradianceMap;
		UniformTex mPrefilterMap;

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
		Sampler mCascadedShadowMapSampler;

		PbrProbe* mProbe;

		AmbientLight* mAmbientLight;
		CascadedShadow* mCascadeShadow;
		DirectionalLight* mLight;
	};

	class PbrForwardPass : public TransformPass
	{
	public:
		PbrForwardPass(CascadedShadow* cascadedShadow);

		void updateConstants(Camera* camera) override;

		void setProbe(PbrProbe* probe);
		void setAmbientLight(AmbientLight* light);
		void setDirLight(DirectionalLight* light);

	private:
		PbrCommonGeometryPass mGeometryPass;
		PbrCommonLightingPass mLightingPass;

		static constexpr unsigned TRANSFORM_BUFFER_BINDINGPOINT = 1;

		static std::vector<std::string> generateDefines(CascadedShadow* cascadedShadow);
	};

	class PbrDeferredGeometryPass : public TransformPass {
	public:
		PbrDeferredGeometryPass();

		void updateConstants(Camera* camera) override;

	private:
		PbrCommonGeometryPass mGeometryPass;
	};

	class PbrDeferredLightingPass : public Pass {
	public:

		PbrDeferredLightingPass(CascadedShadow* cascadedShadow);

		void setAlbedoMap(const Texture* texture);
		void setAoMetalRoughnessMap(const Texture* texture);
		void setNormalEyeMap(const Texture* texture);
		void setNormalizedViewSpaceZMap(const Texture* texture);

		void setInverseProjMatrixFromGPass(const glm::mat4& mat);

		void updateConstants(Camera* camera) override;

		void setProbe(PbrProbe* probe);
		void setAmbientLight(AmbientLight* light);
		void setDirLight(DirectionalLight* light);

	private:
		UniformTex mAlbedoMap;
		UniformTex mAoMetalRoughnessMap;
		UniformTex mNormalEyeMap;
		UniformTex mNormalizedViewSpaceZMap;

		Uniform mInverseProjFromGPass;

		PbrCommonLightingPass mLightingPass;
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