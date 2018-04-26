#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/PBRShader.hpp>
#include <texture/opengl/TextureGL.hpp>

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

	virtual void setIrradianceMap(CubeMap* irradianceMap) override;

	virtual void setLightColor(glm::vec3 color) override;
	virtual void setLightDirection(glm::vec3 direction) override;
	virtual void setLightProjMatrix(glm::mat4 mat) override;
	virtual void setLightSpaceMatrix(glm::mat4 mat) override;
	virtual void setLightViewMatrix(glm::mat4 mat) override;

	virtual void setShadowMap(Texture* texture) override;
	virtual void setSkyBox(CubeMap* sky) override;

	virtual void setCameraPosition(glm::vec3 position) override;
	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::mat4 biasMatrix;
	DirLight dirLight;

	CubeMapGL* irradianceMap;

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