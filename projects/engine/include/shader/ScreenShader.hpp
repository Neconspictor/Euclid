#pragma once
#include <shader/shader.hpp>

class ScreenShader : public ShaderConfig
{
public:
	ScreenShader() {}
	virtual ~ScreenShader() {};

	virtual void useTexture(Texture* texture) = 0;
};