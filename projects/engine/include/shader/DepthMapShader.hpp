#pragma once
#include <shader/shader.hpp>

class CubeDepthMapShader : public Shader
{
public:
	CubeDepthMapShader() : Shader() {}
	virtual ~CubeDepthMapShader() {};

	virtual void useCubeDepthMap(CubeMap* map) = 0;

	virtual void setLightPos(glm::vec3 pos) = 0;

	virtual void setRange(float range) = 0;
};

class DepthMapShader : public Shader
{
public:
	DepthMapShader() : Shader() {}
	virtual ~DepthMapShader() {};

	virtual void useDepthMapTexture(Texture* texture) = 0;
};