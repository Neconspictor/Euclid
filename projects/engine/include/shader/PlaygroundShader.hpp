#ifndef PLAYGROUND_SHADER_HPP
#define PLAYGROUND_SHADER_HPP
#include <shader/Shader.hpp>
#include <string>

class PlaygroundShader : public Shader
{
public:
	PlaygroundShader();
	virtual ~PlaygroundShader();
	
	virtual void setTexture1(const std::string& textureName) = 0;
	
	virtual void setTexture2(const std::string& textureName) = 0;
	
	void setTextureMixValue(float mixValue);

protected:
	float mixValue;
};
#endif