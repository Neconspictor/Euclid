#ifndef PLAYGROUND_SHADER_HPP
#define PLAYGROUND_SHADER_HPP
#include "shader/Shader.hpp"

class PlaygroundShader : public Shader
{
public:
	PlaygroundShader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual ~PlaygroundShader();

	void setTexture1(const std::string& textureName);
	void setTexture2(const std::string& textureName);
	void setTextureMixValue(float mixValue);

	virtual void release() override;
	virtual void use() override;
	virtual void draw(Model const& model, glm::mat4 const& transform) override;

protected:
	GLuint texture, texture2;
	float mixValue;
};
#endif