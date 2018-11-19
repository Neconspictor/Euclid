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
	virtual ~PBRShader_Deferred_GeometryGL();

private:
	glm::mat4 transform;
	glm::mat4 modelView;
	glm::mat3 modelView_normalMatrix;
	//SamplerGL m_sampler;
};

class PBRShader_Deferred_LightingGL : public ShaderGL {
public:

	struct DirLight
	{
		glm::vec3 direction;
		glm::vec3 color;
	};

	PBRShader_Deferred_LightingGL();
	virtual ~PBRShader_Deferred_LightingGL();

	void setBrdfLookupTexture(TextureGL* brdfLUT);

	void setGBuffer(PBR_GBufferGL* gBuffer);

	void setInverseViewFromGPass(glm::mat4 inverseView);

	void setIrradianceMap(CubeMapGL* irradianceMap);

	void setLightColor(glm::vec3 color);
	void setLightDirection(glm::vec3 direction);

	void setPrefilterMap(CubeMapGL* prefilterMap);

	void setShadowMap(TextureGL* texture);
	void setAOMap(TextureGL* texture);
	void setSkyBox(CubeMapGL* sky);

	void setWorldToLightSpaceMatrix(glm::mat4 worldToLight);

	void setCascadedDepthMap(TextureGL* cascadedDepthMap);
	void setCascadedData(CascadedShadowGL::CascadeData* cascadedData);

private:
	PBR_GBufferGL* gBuffer;
	glm::mat4 transform;
	glm::mat4 myView;
	glm::mat4 inverseViewFromGPass;

	TextureGL* brdfLUT;
	DirLight dirWorldToLight;
	DirLight dirEyeToLight;

	CubeMapGL* irradianceMap;
	CubeMapGL* prefilterMap;

	glm::mat4 eyeToLight;
	glm::mat4 worldToLight;

	TextureGL* shadowMap;
	TextureGL* ssaoMap;
	CubeMapGL* skybox;

	glm::mat4 biasMatrix;

	// Cascaded shadow mapping
	TextureGL* cascadedDepthMap;
	GLuint cascadeBufferUBO;

};

class PBR_ConvolutionShaderGL : public ShaderGL
{
public:
	PBR_ConvolutionShaderGL();

	virtual ~PBR_ConvolutionShaderGL();

	void setEnvironmentMap(CubeMapGL* cubeMap);

private:
	UniformTex mCubeMap;
};

class PBR_PrefilterShaderGL : public ShaderGL
{
public:
	PBR_PrefilterShaderGL();

	virtual ~PBR_PrefilterShaderGL();

	void setMapToPrefilter(CubeMapGL* cubeMap);

	void setRoughness(float roughness);

private:
	UniformTex mCubeMap;
	Uniform mRoughness;
};

class PBR_BrdfPrecomputeShaderGL : public ShaderGL
{
public:
	PBR_BrdfPrecomputeShaderGL();

	virtual ~PBR_BrdfPrecomputeShaderGL();

private:
	Uniform mTransform;
};