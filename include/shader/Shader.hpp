#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>
#include <string>
#include <model/Model.hpp>

class Shader
{
public:
	Shader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual void use();
	GLuint getProgramID();
	bool loadingFailed();

	virtual void release();

	virtual void draw(Model const& model, glm::mat4 const& transform);

protected:
	GLuint programID;

private:

	GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile);
	bool loadShaderFromFile(const std::string& file, std::string* shaderContent);
	bool compileShader(const std::string& shaderContent, GLuint shaderResourceID);
};

#endif