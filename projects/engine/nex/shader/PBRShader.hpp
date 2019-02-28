#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/shader/ShaderBuffer.hpp>


namespace nex
{
	class PBRShader : public Shader
	{
	public:

		struct DirLight
		{
			glm::vec3 direction;
			glm::vec3 color;
		};

		PBRShader();

		void setAlbedoMap(const Texture* texture);
		void setAmbientOcclusionMap(const Texture* texture);
		void setEmissionMap(const Texture* texture);
		void setMetalMap(const Texture* texture);
		void setNormalMap(const Texture* texture);
		void setRoughnessMap(const Texture* texture);


		void setBrdfLookupTexture(const Texture* brdfLUT);

		void setIrradianceMap(const CubeMap* irradianceMap);

		void setLightColor(const glm::vec3& color);
		void setLightDirection(const glm::vec3& direction);

		void setLightProjMatrix(const glm::mat4& mat);
		void setLightSpaceMatrix(const glm::mat4& mat);
		void setLightViewMatrix(const glm::mat4& mat);

		void setPrefilterMap(const CubeMap* prefilterMap);

		void setProjectionMatrix(const glm::mat4& mat);

		void setShadowMap(const Texture* texture);

		//TODO
		//void setSkyBox(const CubeMap* sky);

		void setCameraPosition(const glm::vec3& position);

		void setBiasMatrix(const glm::mat4& mat);

		void setModelMatrix(const glm::mat4& mat);
		void setModelViewMatrix(const glm::mat4& mat);
		void setNormalMatrix(const glm::mat3& mat);

		void setMVP(const glm::mat4& mat);

		void setViewMatrix(const glm::mat4& mat);
		void setInverseViewMatrix(const glm::mat4& mat);

		void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;
		void onMaterialUpdate(const Material* material) override;

	private:

		UniformTex mAlbedoMap;
		UniformTex mAmbientOcclusionMap;
		UniformTex mEmissionMap;
		UniformTex mMetalMap;
		UniformTex mNormalMap;
		UniformTex mRoughnessMap;

		Uniform mBiasMatrix;
		glm::mat4 mBiasMatrixSource;
		UniformTex mBrdfLUT;

		UniformTex mIrradianceMap;
		UniformTex mPrefilterMap;

		Uniform mLightDirection;
		Uniform mLightColor;
		Uniform mLightProjMatrix;
		Uniform mLightSpaceMatrix;
		Uniform mLightViewMatrix;
		Uniform mModelMatrix;
		Uniform mModelView;
		Uniform mNormalMatrix;

		UniformTex mShadowMap;
		UniformTex mSkybox;

		Uniform mTransform;
		Uniform mCameraPos;

		Uniform mView;
		Uniform mInverseView;

		glm::mat4 const* mProjectionMatrixSource;
		glm::mat4 const* mViewMatrixSource;
	};

	class PBRShader_Deferred_Geometry : public Shader {
	public:
		PBRShader_Deferred_Geometry();

		void setAlbedoMap(const Texture* texture);
		void setAmbientOcclusionMap(const Texture* texture);
		void setEmissionMap(const Texture* texture);
		void setMetalMap(const Texture* texture);
		void setNormalMap(const Texture* texture);
		void setRoughnessMap(const Texture* texture);


		void setMVP(const glm::mat4& mat);
		void setModelViewMatrix(const glm::mat4& mat);
		void setModelView_NormalMatrix(const glm::mat4& mat);

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);


		void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;
		void onMaterialUpdate(const Material* material) override;

	private:

		Uniform mTransform;
		Uniform mModelView;
		Uniform mModelView_normalMatrix;

		UniformTex mAlbedoMap;
		UniformTex mAmbientOcclusionMap;
		UniformTex mEmissionMap;
		UniformTex mMetalMap;
		UniformTex mNormalMap;
		UniformTex mRoughnessMap;

		glm::mat4 const* mProjection;
		glm::mat4 const* mView;
	};

	class PBRShader_Deferred_Lighting : public TransformShader {
	public:

		PBRShader_Deferred_Lighting(unsigned csmNumCascades,
			const CascadedShadow::PCFFilter& pcf);

		virtual ~PBRShader_Deferred_Lighting();


		void setMVP(const glm::mat4& trafo);
		void setViewGPass(const glm::mat4& mat);
		void setInverseViewFromGPass(const glm::mat4& mat);

		void setBrdfLookupTexture(const Texture* brdfLUT);

		void setWorldLightDirection(const glm::vec3& direction);
		void setEyeLightDirection(const glm::vec3& direction);
		void setLightColor(const glm::vec3& color);



		void setIrradianceMap(const CubeMap* irradianceMap);
		void setPrefilterMap(const CubeMap* prefilterMap);

		void setEyeToLightSpaceMatrix(const glm::mat4& mat);
		void setWorldToLightSpaceMatrix(const glm::mat4& mat);

		void setShadowMap(const Texture* texture);
		void setAOMap(const Texture* texture);
		void setSkyBox(const CubeMap* sky);

		void setCascadedDepthMap(const Texture* cascadedDepthMap);
		void setCascadedData(const CascadedShadow::CascadeData* cascadedData);

		void setAlbedoMap(const Texture* texture);
		void setAoMetalRoughnessMap(const Texture* texture);
		void setNormalEyeMap(const Texture* texture);
		void setPositionEyeMap(const Texture* texture);
		void setDepthMap(const Texture* texture);

		void setInverseProjMatrixFromGPass(const glm::mat4& mat);


		void onTransformUpdate(const TransformData& data) override;

	private:
		Uniform mTransform;
		Uniform mViewGPass;
		Uniform mInverseViewFromGPass;

		UniformTex mBrdfLUT;

		//Uniform mWorldDirection;
		Uniform mEyeLightDirection;
		Uniform mLightColor;

		UniformTex mIrradianceMap;
		UniformTex mPrefilterMap;

		Uniform mEyeToLightTrafo;
		Uniform mWorldToLightTrafo;

		UniformTex mShadowMap;
		UniformTex mAoMap;
		UniformTex mSkyBox;

		Uniform mBiasMatrix;
		glm::mat4 mBiasMatrixSource;

		// Cascaded shadow mapping
		UniformTex mCascadedDepthMap;
		UniformBuffer cascadeBufferUBO;


		UniformTex mAlbedoMap;
		UniformTex mAoMetalRoughnessMap;
		UniformTex mNormalEyeMap;
		UniformTex mDepthMap;

		Uniform mInverseProjFromGPass;

		// CSM
		unsigned mCsmNumCascades;
		CascadedShadow::PCFFilter mCsmPcf;

		std::vector<std::string> generateCsmDefines();
		
		template<typename T>
		std::string makeDefine(const char* str, T value);
	};

	template <typename T>
	std::string PBRShader_Deferred_Lighting::makeDefine(const char* str, T value)
	{
		std::stringstream ss;
		ss <<"#define " << str << " " << value;
		return ss.str();
	}

	class PBR_ConvolutionShader : public Shader
	{
	public:
		PBR_ConvolutionShader();

		virtual ~PBR_ConvolutionShader() = default;

		void setProjection(const glm::mat4& mat);
		void setView(const glm::mat4& mat);
		void setEnvironmentMap(const CubeMap* cubeMap);

	private:
		Uniform mProjection;
		Uniform mView;
		UniformTex mEnvironmentMap;
	};

	class PBR_PrefilterShader : public Shader
	{
	public:
		PBR_PrefilterShader();

		virtual ~PBR_PrefilterShader() = default;

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

	class PBR_BrdfPrecomputeShader : public TransformShader
	{
	public:
		PBR_BrdfPrecomputeShader();

		virtual ~PBR_BrdfPrecomputeShader() = default;

		void setMVP(const glm::mat4& mat);

		void onTransformUpdate(const TransformData& data) override;
	private:
		Uniform mTransform;
	};
}
