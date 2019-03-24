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

			void setModelViewMatrix(const glm::mat4& mat);
			void setTransform(const glm::mat4& mat);

			void setProjection(const glm::mat4& mat);
			void setView(const glm::mat4& mat);

			void setNearFarPlane(const glm::vec2& nearFarPlane);

			/**
			 * Prerequisites: prjection and view matrix are set (and mustn't be null!) 
			 */
			void doModelMatrixUpdate(const glm::mat4& model);

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

			Uniform mModelView;
			Uniform mTransform;
			Uniform mNearFarPlane;

			glm::mat4 const* mProjectionMatrixSource;
			glm::mat4 const* mViewMatrixSource;

			ShaderProgram* mProgram;
		};

		class CommonLightingMaterial
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
			CommonLightingMaterial(const CascadedShadow& cascadedShadow);
			void init(ShaderProgram* program);

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

		void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;
		void onMaterialUpdate(const Material* material) override;
	};

	class PBRShader_Deferred_Geometry : public Shader, public pbr::CommonGeometryMaterial {
	public:
		PBRShader_Deferred_Geometry();

		void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;
		void onMaterialUpdate(const Material* material) override;
	};

	class PBRShader_Deferred_Lighting : public TransformShader, public pbr::CommonLightingMaterial {
	public:

		PBRShader_Deferred_Lighting(const CascadedShadow& cascadedShadow);

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
