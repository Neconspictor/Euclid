#pragma once
#include <shader/shader.hpp>

class DepthMapShader : public Shader
{
public:
	DepthMapShader() : Shader() {}
	virtual ~DepthMapShader() {};

	virtual void useDepthMapTexture(Texture* texture) = 0;
};