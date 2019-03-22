#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/shader/ShaderBuffer.hpp>


namespace nex
{
	namespace pbr
	{
		class CommonGeometryMaterial
		{
		public:
			void setAlbedoMap(const Texture* texture);
			void setAmbientOcclusionMap(const Texture* texture);
			void setMetalMap(const Texture* texture);
			void setNormalMap(const Texture* texture);
			void setRoughnessMap(const Texture* texture);

		protected:
			CommonGeometryMaterial();
			void init(ShaderProgram* program);

		private:
			// mesh material
			UniformTex mAlbedoMap;
			UniformTex mAmbientOcclusionMap;
			UniformTex mMetalMap;
			UniformTex mNormalMap;
			UniformTex mRoughnessMap;
			ShaderProgram* mProgram;
		};

		class CommonLightingMaterial
		{
		public:
			void setGlobalAOMap(const Texture* texture);

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

			void setNearFarPlane(const glm::vec2& nearFarPlane);



		protected:
			CommonLightingMaterial(const CascadedShadow& cascadedShadow);
			void init(ShaderProgram* program);

		private:
			// ao
			UniformTex mGlobalAo;

			//ibl
			UniformTex mBrdfLUT;
			UniformTex mIrradianceMap;
			UniformTex mPrefilterMap;

			// CSM
			UniformTex mCascadedDepthMap;
			ShaderStorageBuffer cascadeBufferUBO; //UniformBuffer ShaderStorageBuffer
			unsigned mCsmNumCascades;
			CascadedShadow::PCFFilter mCsmPcf;
			bool mCsmEnabled;
			float mCsmBiasMultiplier;


			Uniform mEyeLightDirection;
			Uniform mLightColor;
			Uniform mLightPower;
			Uniform mAmbientLightPower;
			Uniform mShadowStrength;

			Uniform mNearFarPlane;

			ShaderProgram* mProgram;
		};
	}

	class PBRShader : public Shader, public pbr::CommonGeometryMaterial, public pbr::CommonLightingMaterial
	{
	public:

		struct DirLight
		{
			glm::vec3 direction;
			glm::vec3 color;
		};

		PBRShader(const CascadedShadow& cascadedShadow);

		void setProjectionMatrix(const glm::mat4& mat);

		void setModelViewMatrix(const glm::mat4& mat);

		void setMVP(const glm::mat4& mat);

		void setViewMatrix(const glm::mat4& mat);
		void setInverseViewMatrix(const glm::mat4& mat);

		void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;
		void onMaterialUpdate(const Material* material) override;

	private:
		Uniform mLightDirection;
		Uniform mLightColor;
		Uniform mLightProjMatrix;
		Uniform mLightSpaceMatrix;
		Uniform mLightViewMatrix;
		Uniform mModelView;

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
		void setNearFarPlane(const glm::vec2& nearFarPlane);

	private:

		Uniform mTransform;
		Uniform mModelView;
		Uniform mModelView_normalMatrix;
		Uniform mNearFarPlane;

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

		PBRShader_Deferred_Lighting(const CascadedShadow& cascadedShadow);

		virtual ~PBRShader_Deferred_Lighting();


		void setMVP(const glm::mat4& trafo);
		void setViewGPass(const glm::mat4& mat);
		void setInverseViewFromGPass(const glm::mat4& mat);

		void setBrdfLookupTexture(const Texture* brdfLUT);

		void setEyeLightDirection(const glm::vec3& direction);
		void setLightColor(const glm::vec3& color);
		void setLightPower(float power);
		void setAmbientLightPower(float power);
		void setShadowStrength(float strength);



		void setIrradianceMap(const CubeMap* irradianceMap);
		void setPrefilterMap(const CubeMap* prefilterMap);

		void setEyeToLightSpaceMatrix(const glm::mat4& mat);
		void setWorldToLightSpaceMatrix(const glm::mat4& mat);

		void setShadowMap(const Texture* texture);
		void setAOMap(const Texture* texture);
		void setSkyBox(const CubeMap* sky);

		void setCascadedDepthMap(const Texture* cascadedDepthMap);
		void setCascadedData(const CascadedShadow::CascadeData& cascadedData, Camera* camera);
		void setCascadedData(ShaderStorageBuffer* buffer);

		void setAlbedoMap(const Texture* texture);
		void setAoMetalRoughnessMap(const Texture* texture);
		void setNormalEyeMap(const Texture* texture);
		void setNormalizedViewSpaceZMap(const Texture* texture);

		void setInverseProjMatrixFromGPass(const glm::mat4& mat);

		void setNearFarPlane(const glm::vec2& nearFarPlane);


		void onTransformUpdate(const TransformData& data) override;

	private:
		Uniform mTransform;
		Uniform mViewGPass;
		Uniform mInverseViewFromGPass;

		UniformTex mBrdfLUT;

		//Uniform mWorldDirection;
		Uniform mEyeLightDirection;
		Uniform mLightColor;
		Uniform mLightPower;
		Uniform mAmbientLightPower;
		Uniform mShadowStrength;

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
		ShaderStorageBuffer cascadeBufferUBO; //UniformBuffer ShaderStorageBuffer


		UniformTex mAlbedoMap;
		UniformTex mAoMetalRoughnessMap;
		UniformTex mNormalEyeMap;
		UniformTex mNormalizedViewSpaceZMap;

		Uniform mInverseProjFromGPass;
		Uniform mNearFarPlane;

		// CSM
		unsigned mCsmNumCascades;
		CascadedShadow::PCFFilter mCsmPcf;
		bool mCsmEnabled;
		float mCsmBiasMultiplier;

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
