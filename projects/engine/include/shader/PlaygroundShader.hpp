#pragma once
#include <shader/Shader.hpp>
#include <string>

class PlaygroundShader : public Shader
{
public:
	PlaygroundShader() {};
	virtual ~PlaygroundShader() {};
	
	virtual void setTexture1(const std::string& textureName) = 0;
	
	virtual void setTexture2(const std::string& textureName) = 0;
	
	virtual void setTextureMixValue(float mixValue) = 0;
};