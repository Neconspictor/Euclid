#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/shader/ShaderBuffer.hpp>
#include <nex/texture/Sampler.hpp>


namespace nex
{
	class PbrCommonGeometryPass
	{
	public:

		static const unsigned ALBEDO_BINDING_POINT = 0;
		static const unsigned AO_BINDING_POINT = 1;
		static const unsigned METALLIC_BINDING_POINT = 2;
		static const unsigned NORMAL_BINDING_POINT = 3;
		static const unsigned ROUGHNESS_BINDING_POINT = 4;

		void setModelViewMatrix(const glm::mat4& mat);
		void setTransform(const glm::mat4& mat);

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);

		void setNearFarPlane(const glm::vec2& nearFarPlane);

		/**
		 * Prerequisites: projection and view matrix are set (and mustn't be null!) 
		 */
		void doModelMatrixUpdate(const glm::mat4& model);

		void updateConstants();

	protected:
		PbrCommonGeometryPass();
		void init(Shader* program);

	private:
		// mesh material
		UniformTex mAlbedoMap;
		UniformTex mAmbientOcclusionMap;
		UniformTex mMetalMap;
		UniformTex mNormalMap;
		UniformTex mRoughnessMap;

		Uniform mModelView;
		Uniform mTransform;
		Uniform mNearFarPlane;

		glm::mat4 const* mProjectionMatrixSource;
		glm::mat4 const* mViewMatrixSource;

		Shader* mProgram;
		Sampler* mDefaultImageSampler;
	};

	class PbrCommonLightingPass
	{
	public:
		void setBrdfLookupTexture(const Texture* brdfLUT);
		void setIrradianceMap(const CubeMap* irradianceMap);
		void setPrefilterMap(const CubeMap* prefilterMap);

		void setCascadedDepthMap(const Texture* cascadedDepthMap);
		void setCascadedData(const CascadedShadow::CascadeData& cascadedData, Camera* camera);
		void setCascadedData(ShaderStorageBuffer* buffer);
		
		
		void setEyeLightDirection(const glm::vec3& direction);
		void setLightColor(const glm::vec3& color);
		void setLightPower(float power);
		void setAmbientLightPower(float power);
		void setShadowStrength(float strength);

		void setInverseViewMatrix(const glm::mat4& mat);

		void setNearFarPlane(const glm::vec2& nearFarPlane);



	protected:
		PbrCommonLightingPass(const CascadedShadow& cascadedShadow);
		void init(Shader* program);

	private:
		//ibl
		UniformTex mBrdfLUT;
		UniformTex mIrradianceMap;
		UniformTex mPrefilterMap;

		// CSM
		UniformTex mCascadedDepthMap;
		ShaderStorageBuffer cascadeBufferUBO; //UniformBuffer ShaderStorageBuffer


		Uniform mEyeLightDirection;
		Uniform mLightColor;
		Uniform mLightPower;
		Uniform mAmbientLightPower;
		Uniform mShadowStrength;

		Uniform mInverseView;

		Uniform mNearFarPlane;

		Shader* mProgram;
		Sampler mSampler;
		Sampler mCascadedShadowMapSampler;
	};

	class PbrForwardPass : public Pass, public PbrCommonGeometryPass, public PbrCommonLightingPass
	{
	public:

		struct DirLight
		{
			glm::vec3 direction;
			glm::vec3 color;
		};

		PbrForwardPass(const CascadedShadow& cascadedShadow);

		void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;
		void onMaterialUpdate(const Material* material) override;
		void updateConstants() override;
	};

	class PbrDeferredGeometryPass : public Pass, public PbrCommonGeometryPass {
	public:
		PbrDeferredGeometryPass();

		void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;
		void onMaterialUpdate(const Material* material) override;
		void updateConstants() override;
	};

	class PbrDeferredLightingPass : public TransformPass, public PbrCommonLightingPass {
	public:

		PbrDeferredLightingPass(const CascadedShadow& cascadedShadow);

		void setMVP(const glm::mat4& trafo);

		void setAlbedoMap(const Texture* texture);
		void setAoMetalRoughnessMap(const Texture* texture);
		void setNormalEyeMap(const Texture* texture);
		void setNormalizedViewSpaceZMap(const Texture* texture);

		void setInverseProjMatrixFromGPass(const glm::mat4& mat);


		void onTransformUpdate(const TransformData& data) override;

	private:
		Uniform mTransform;

		UniformTex mAlbedoMap;
		UniformTex mAoMetalRoughnessMap;
		UniformTex mNormalEyeMap;
		UniformTex mNormalizedViewSpaceZMap;

		Uniform mInverseProjFromGPass;
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

	class PbrBrdfPrecomputePass : public TransformPass
	{
	public:
		PbrBrdfPrecomputePass();

		virtual ~PbrBrdfPrecomputePass() = default;

		void setMVP(const glm::mat4& mat);

		void onTransformUpdate(const TransformData& data) override;
	private:
		Uniform mTransform;
	};
}