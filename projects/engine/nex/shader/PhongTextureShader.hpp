#pragma once
#include <nex/shader/shader.hpp>

class CubeMap;

class PhongTextureShader : public ShaderConfig
{
public:
	virtual ~PhongTextureShader() {};

	virtual const glm::vec3& getLightColor() const = 0;
	virtual const glm::vec3& getLightPosition() const = 0;

	virtual void setLightColor(glm::vec3 color) = 0;
	virtual void setLightDirection(glm::vec3 direction) = 0;

	// NOTE: positions is assumed to be an array with 4 elements!
	virtual void setLightProjMatrix(glm::mat4 mat) = 0;
	virtual void setLightSpaceMatrix(glm::mat4 mat) = 0;
	virtual void setLightViewMatrix(glm::mat4 mat) = 0;
	virtual void setPointLightPositions(glm::vec3* positions) = 0;
	virtual void setPointLightRange(float range) = 0;
	virtual void setPointLightShadowMap(CubeDepthMap* map) = 0;
	virtual void setSkyBox(CubeMap* sky) = 0;
	virtual void setShadowMap(Texture* texture) = 0;
	virtual void setSpotLightDirection(glm::vec3 direction) = 0;
	virtual void setVarianceShadowMap(Texture* texture) = 0;
	virtual void setViewPosition(glm::vec3 position) = 0;
};