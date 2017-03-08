#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/PhongTextureShader.hpp>
#include <texture/opengl/TextureGL.hpp>

class PhongTexShaderGL : public PhongTextureShader, public ShaderConfigGL
{
public:
	PhongTexShaderGL();

	virtual ~PhongTexShaderGL();
	virtual const glm::vec3& getLightColor() const override;
	virtual const glm::vec3& getLightPosition() const override;
	virtual void setLightColor(glm::vec3 color) override;
	virtual void setLightDirection(glm::vec3 direction) override;
	virtual void setLightSpaceMatrix(glm::mat4 mat) override;
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

	ShaderAttributeCollection attributes;
	glm::vec3 dirLightDirection;
	GLuint instancedShaderProgram;
	glm::vec3 lightColor;
	glm::mat4 lightSpaceMatrix;
	glm::mat4 modelView;
	glm::mat3 normalMatrix;
	glm::vec3 pointLightPositions[4];
	float pointLightRange;
	CubeDepthMapGL* pointLightShadowMap;
	glm::vec3 spotLightDirection;
	TextureGL* shadowMap;
	CubeMapGL* skybox;
	glm::mat4 transform;
	glm::vec3 viewPosition;
	TextureGL* vsMap;
};