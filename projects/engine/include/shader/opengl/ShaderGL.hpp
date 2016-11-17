#ifndef ENGINE_SHADER_OPENGL_SHADERGL_HPP
#define ENGINE_SHADER_OPENGL_SHADERGL_HPP
#include <shader/Shader.hpp>
#include <GL/glew.h>
#include <string>

class ShaderGL : public Shader
{
public:
	ShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	virtual ~ShaderGL();

	static bool compileShader(const std::string& shaderContent, GLuint shaderResourceID);
	void draw(Model const& model, glm::mat4 const& transform) override;
	GLuint getProgramID();
	bool loadingFailed() override;
	static GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile);
	void release() override;
	void use() override;

protected:
	GLuint programID;
};

#endif