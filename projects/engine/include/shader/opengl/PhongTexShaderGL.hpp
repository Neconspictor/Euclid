#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/PhongTextureShader.hpp>
#include <texture/opengl/TextureGL.hpp>

class PhongTexShaderGL : public PhongTextureShader, public ShaderConfigGL
{
public:

	struct DirLight
	{
		glm::vec3 direction;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
	};

	struct PointLight
	{
		glm::vec3 position;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		float constant;
		float linear;
		float quadratic;
		float range;
	};

	struct SpotLight : PointLight
	{
		glm::vec3 direction;
		float cutOff;
		float outerCutOff;
	};

	PhongTexShaderGL();

	virtual ~PhongTexShaderGL();
	virtual const glm::vec3& getLightColor() const override;
	virtual const glm::vec3& getLightPosition() const override;
	virtual void setLightColor(glm::vec3 color) override;
	virtual void setLightDirection(glm::vec3 direction) override;
	virtual void setLightProjMatrix(glm::mat4 mat) override;
	virtual void setLightSpaceMatrix(glm::mat4 mat) override;
	virtual void setLightViewMatrix(glm::mat4 mat) override;
	virtual void setPointLightPositions(glm::vec3* positions) override;
	virtual void setPointLightRange(float range) override;
	virtual void setPointLightShadowMap(CubeDepthMap* map) override;
	virtual void setShadowMap(Texture* texture) override;
	virtual void setSkyBox(CubeMap* sky) override;
	virtual void setSpotLightDirection(glm::vec3 direction) override;
	virtual void setVarianceShadowMap(Texture* texture) override;
	virtual void setViewPosition(glm::vec3 position) override;
	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	glm::mat4 biasMatrix;
	DirLight dirLight;
	glm::vec3 lightColor;
	glm::mat4 lightProjMatrix;
	glm::mat4 lightSpaceMatrix;
	glm::mat4 lightViewMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 modelView;
	glm::mat3 normalMatrix;
	PointLight pointLights[4];
	float pointLightRange;
	CubeDepthMapGL* pointLightShadowMap;
	TextureGL* shadowMap;
	CubeMapGL* skybox;
	SpotLight spotLight;
	glm::mat4 transform;
	glm::vec3 viewPosition;
	TextureGL* vsMap;
};