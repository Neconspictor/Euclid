#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class PhongTexShaderGL : public ShaderConfigGL
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
	const glm::vec3& getLightColor() const;
	const glm::vec3& getLightPosition() const;
	void setLightColor(glm::vec3 color);
	void setLightDirection(glm::vec3 direction);
	void setLightProjMatrix(glm::mat4 mat);
	void setLightSpaceMatrix(glm::mat4 mat);
	void setLightViewMatrix(glm::mat4 mat);
	void setPointLightPositions(glm::vec3* positions);
	void setPointLightRange(float range);
	void setPointLightShadowMap(CubeDepthMapGL* map);
	void setShadowMap(TextureGL* texture);
	void setSkyBox(CubeMapGL* sky);
	void setSpotLightDirection(glm::vec3 direction);
	void setVarianceShadowMap(TextureGL* texture);
	void setViewPosition(glm::vec3 position);
	void update(const MeshGL& mesh, const TransformData& data) override;

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