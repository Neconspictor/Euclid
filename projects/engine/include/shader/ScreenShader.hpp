#pragma once
#include <shader/shader.hpp>

class ScreenShader : public Shader
{
public:
	ScreenShader() {}
	virtual ~ScreenShader() {};

	virtual void useTexture(Texture* texture) = 0;
};