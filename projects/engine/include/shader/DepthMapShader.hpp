#pragma once
#include <shader/shader.hpp>

class CubeDepthMapShader : public Shader
{
public:
	CubeDepthMapShader() : Shader() {}
	virtual ~CubeDepthMapShader() {};

	virtual void useCubeDepthMap(CubeMap* map) = 0;
};

class DepthMapShader : public Shader
{
public:
	DepthMapShader() : Shader() {}
	virtual ~DepthMapShader() {};

	virtual void useDepthMapTexture(Texture* texture) = 0;
};