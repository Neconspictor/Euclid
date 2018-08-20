#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/shader/PBRShader.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class PBRShaderGL : public PBRShader, public ShaderConfigGL
{
public:

	struct DirLight
	{
		glm::vec3 direction;
		glm::vec3 color;
	};

	PBRShaderGL();

	virtual ~PBRShaderGL();
	virtual const glm::vec3& getLightColor() const override;
	virtual const glm::vec3& getLightPosition() const override;

	virtual void setBrdfLookupTexture(Texture* brdfLUT) override;

	virtual void setIrradianceMap(CubeMap* irradianceMap) override;

	virtual void setLightColor(glm::vec3 color) override;
	virtual void setLightDirection(glm::vec3 direction) override;
	virtual void setLightProjMatrix(glm::mat4 mat) override;
	virtual void setLightSpaceMatrix(glm::mat4 mat) override;
	virtual void setLightViewMatrix(glm::mat4 mat) override;

	virtual void setPrefilterMap(CubeMap* prefilterMap) override;

	virtual void setShadowMap(Texture* texture) override;
	virtual void setSkyBox(CubeMap* sky) override;

	virtual void setCameraPosition(glm::vec3 position) override;
	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::mat4 biasMatrix;
	Texture* brdfLUT;
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

class PBRShader_Deferred_GeometryGL : public PBRShader_Deferred_Geometry, public ShaderConfigGL {
public:
	PBRShader_Deferred_GeometryGL();
	virtual ~PBRShader_Deferred_GeometryGL();

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::mat4 transform;
	glm::mat4 modelView;
	glm::mat3 modelView_normalMatrix;
};

class PBRShader_Deferred_LightingGL : public PBRShader_Deferred_Lighting, public ShaderConfigGL {
public:

	struct DirLight
	{
		glm::vec3 direction;
		glm::vec3 color;
	};

	PBRShader_Deferred_LightingGL();
	virtual ~PBRShader_Deferred_LightingGL();

	virtual void setBrdfLookupTexture(Texture* brdfLUT) override;

	virtual void setGBuffer(PBR_GBuffer* gBuffer);

	virtual void setInverseViewFromGPass(glm::mat4 inverseView) override;

	virtual void setIrradianceMap(CubeMap* irradianceMap) override;

	virtual void setLightColor(glm::vec3 color) override;
	virtual void setLightDirection(glm::vec3 direction) override;

	virtual void setPrefilterMap(CubeMap* prefilterMap) override;

	virtual void setShadowMap(Texture* texture) override;
	virtual void setSSAOMap(Texture* texture) override;
	virtual void setSkyBox(CubeMap* sky) override;

	virtual void setWorldToLightSpaceMatrix(glm::mat4 worldToLight) override;

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	PBR_GBuffer* gBuffer;
	glm::mat4 transform;
	glm::mat4 myView;
	glm::mat4 inverseViewFromGPass;

	Texture* brdfLUT;
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

};

class PBR_ConvolutionShaderGL : public PBR_ConvolutionShader, public ShaderConfigGL
{
public:
	PBR_ConvolutionShaderGL();

	virtual ~PBR_ConvolutionShaderGL();

	virtual void setEnvironmentMap(CubeMap* cubeMap) override;

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	CubeMapGL* cubeMap;
};

class PBR_PrefilterShaderGL : public PBR_PrefilterShader, public ShaderConfigGL
{
public:
	PBR_PrefilterShaderGL();

	virtual ~PBR_PrefilterShaderGL();

	virtual void setMapToPrefilter(CubeMap* cubeMap) override;

	virtual void setRoughness(float roughness) override;

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	CubeMapGL* cubeMap;
	float roughness;
};

class PBR_BrdfPrecomputeShaderGL : public PBR_BrdfPrecomputeShader, public ShaderConfigGL
{
public:
	PBR_BrdfPrecomputeShaderGL();

	virtual ~PBR_BrdfPrecomputeShaderGL();

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::mat4 transform;
};