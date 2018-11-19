#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/shadowing/CascadedShadowGL.hpp>

class PBRShaderGL : public ShaderGL
{
public:

	struct DirLight
	{
		glm::vec3 direction;
		glm::vec3 color;
	};

	PBRShaderGL();

	virtual ~PBRShaderGL() = default;

	void setAlbedoMap(const TextureGL* texture);
	void setAmbientOcclusionMap(const TextureGL* texture);
	void setEmissionMap(const TextureGL* texture);
	void setMetalMap(const TextureGL* texture);
	void setNormalMap(const TextureGL* texture);
	void setRoughnessMap(const TextureGL* texture);


	void setBrdfLookupTexture(const TextureGL* brdfLUT);

	void setIrradianceMap(const CubeMapGL* irradianceMap);

	void setLightColor(const glm::vec3& color);
	void setLightDirection(const glm::vec3& direction);

	void setLightProjMatrix(const glm::mat4& mat);
	void setLightSpaceMatrix(const glm::mat4& mat);
	void setLightViewMatrix(const glm::mat4& mat);

	void setPrefilterMap(const CubeMapGL* prefilterMap);

	void setShadowMap(const TextureGL* texture);

	//TODO
	//void setSkyBox(const CubeMapGL* sky);

	void setCameraPosition(const glm::vec3& position);

	void setBiasMatrix(const glm::mat4& mat);

	void setModelMatrix(const glm::mat4& mat);
	void setModelViewMatrix(const glm::mat4& mat);
	void setNormalMatrix(const glm::mat3& mat);

	void setMVP(const glm::mat4& mat);

	void setInverseViewMatrix(const glm::mat4& mat);

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
};

class PBRShader_Deferred_GeometryGL : public ShaderGL {
public:
	PBRShader_Deferred_GeometryGL();
	virtual ~PBRShader_Deferred_GeometryGL() = default;

	void setAlbedoMap(const TextureGL* texture);
	void setAmbientOcclusionMap(const TextureGL* texture);
	void setEmissionMap(const TextureGL* texture);
	void setMetalMap(const TextureGL* texture);
	void setNormalMap(const TextureGL* texture);
	void setRoughnessMap(const TextureGL* texture);

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
};

class PBRShader_Deferred_LightingGL : public ShaderGL {
public:

	PBRShader_Deferred_LightingGL();
	virtual ~PBRShader_Deferred_LightingGL();


	void setMVP(const glm::mat4& trafo);
	void setViewGPass(const glm::mat4& mat);
	void setInverseViewFromGPass(const glm::mat4& mat);

	void setBrdfLookupTexture(const TextureGL* brdfLUT);

	void setWorldLightDirection(const glm::vec3& direction);
	void setEyeLightDirection(const glm::vec3& direction);
	void setLightColor(const glm::vec3& color);


	
	void setIrradianceMap(const CubeMapGL* irradianceMap);
	void setPrefilterMap(const CubeMapGL* prefilterMap);

	void setEyeToLightSpaceMatrix(const glm::mat4& mat);
	void setWorldToLightSpaceMatrix(const glm::mat4& mat);

	void setShadowMap(const TextureGL* texture);
	void setAOMap(const TextureGL* texture);
	void setSkyBox(const CubeMapGL* sky);

	void setCascadedDepthMap(const TextureGL* cascadedDepthMap);
	void setCascadedData(const CascadedShadowGL::CascadeData* cascadedData);

	void setAlbedoMap(const TextureGL* texture);
	void setAoMetalRoughnessMap(const TextureGL* texture);
	void setNormalEyeMap(const TextureGL* texture);
	void setPositionEyeMap(const TextureGL* texture);



private:
	Uniform mTransform;
	Uniform mViewGPass;
	Uniform mInverseViewFromGPass;

	UniformTex mBrdfLUT;

	Uniform mWorldDirection;
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
	GLuint cascadeBufferUBO;


	UniformTex mAlbedoMap;
	UniformTex mAoMetalRoughnessMap;
	UniformTex mNormalEyeMap;
	UniformTex mPositionEyeMap;
};

class PBR_ConvolutionShaderGL : public ShaderGL
{
public:
	PBR_ConvolutionShaderGL();

	virtual ~PBR_ConvolutionShaderGL() = default;

	void setProjection(const glm::mat4& mat);
	void setView(const glm::mat4& mat);
	void setEnvironmentMap(const CubeMapGL* cubeMap);

private:
	Uniform mProjection;
	Uniform mView;
	UniformTex mEnvironmentMap;
};

class PBR_PrefilterShaderGL : public ShaderGL
{
public:
	PBR_PrefilterShaderGL();

	virtual ~PBR_PrefilterShaderGL() = default;

	void setMapToPrefilter(CubeMapGL* cubeMap);

	void setRoughness(float roughness);

	void setProjection(const glm::mat4& mat);
	void setView(const glm::mat4& mat);

private:
	Uniform mProjection;
	Uniform mView;
	UniformTex mEnvironmentMap;
	Uniform mRoughness;
};

class PBR_BrdfPrecomputeShaderGL : public ShaderGL
{
public:
	PBR_BrdfPrecomputeShaderGL();

	virtual ~PBR_BrdfPrecomputeShaderGL() = default;

	void setMVP(const glm::mat4& mat);

private:
	Uniform mTransform;
};