#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/shadowing/CascadedShadowGL.hpp>

class PBRShaderGL : public ShaderConfigGL
{
public:

	struct DirLight
	{
		glm::vec3 direction;
		glm::vec3 color;
	};

	PBRShaderGL();

	virtual ~PBRShaderGL();
	const glm::vec3& getLightColor() const;
	const glm::vec3& getLightPosition() const;

	void setBrdfLookupTexture(TextureGL* brdfLUT);

	void setIrradianceMap(CubeMapGL* irradianceMap);

	void setLightColor(glm::vec3 color);
	void setLightDirection(glm::vec3 direction);
	void setLightProjMatrix(glm::mat4 mat);
	void setLightSpaceMatrix(glm::mat4 mat);
	void setLightViewMatrix(glm::mat4 mat);

	void setPrefilterMap(CubeMapGL* prefilterMap);

	void setShadowMap(TextureGL* texture);
	void setSkyBox(CubeMapGL* sky);

	void setCameraPosition(glm::vec3 position);
	void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::mat4 biasMatrix;
	TextureGL* brdfLUT;
	DirLight dirLight;

	CubeMapGL* irradianceMap;
	CubeMapGL* prefilterMap;

	glm::vec3 lightColor;
	glm::mat4 lightProjMatrix;
	glm::mat4 lightSpaceMatrix;
	glm::mat4 lightViewMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 modelView;
	glm::mat3 normalMatrix;

	TextureGL* shadowMap;
	CubeMapGL* skybox;

	glm::mat4 transform;
	glm::vec3 cameraPos;

	glm::mat4 inverseView;
};

class PBRShader_Deferred_GeometryGL : public ShaderConfigGL {
public:
	PBRShader_Deferred_GeometryGL();
	virtual ~PBRShader_Deferred_GeometryGL();

	void update(const MeshGL& mesh, const TransformData& data) override;

	void beforeDrawing(const MeshGL& mesh) override;
	void afterDrawing(const MeshGL& mesh) override;

private:
	glm::mat4 transform;
	glm::mat4 modelView;
	glm::mat3 modelView_normalMatrix;
	//SamplerGL m_sampler;
};

class PBRShader_Deferred_LightingGL : public ShaderConfigGL {
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

	void update(const MeshGL& mesh, const TransformData& data);

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

class PBR_ConvolutionShaderGL : public ShaderConfigGL
{
public:
	PBR_ConvolutionShaderGL();

	virtual ~PBR_ConvolutionShaderGL();

	void setEnvironmentMap(CubeMapGL* cubeMap);

	void update(const MeshGL& mesh, const TransformData& data) override;

private:
	CubeMapGL* cubeMap;
};

class PBR_PrefilterShaderGL : public ShaderConfigGL
{
public:
	PBR_PrefilterShaderGL();

	virtual ~PBR_PrefilterShaderGL();

	void setMapToPrefilter(CubeMapGL* cubeMap);

	void setRoughness(float roughness);

	void update(const MeshGL& mesh, const TransformData& data) override;

private:
	CubeMapGL* cubeMap;
	float roughness;
};

class PBR_BrdfPrecomputeShaderGL : public ShaderConfigGL
{
public:
	PBR_BrdfPrecomputeShaderGL();

	virtual ~PBR_BrdfPrecomputeShaderGL();

	void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::mat4 transform;
};