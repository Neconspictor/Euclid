#ifndef ENGINE_SHADER_OPENGL_SAHDERGL_HPP
#define ENGINE_SHADER_OPENGL_SAHDERGL_HPP
#include <shader/Shader.hpp>
#include <GL/glew.h>
#include <string>

class ShaderGL : public Shader
{
	 ShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	 virtual ~ShaderGL();

public:
	void draw(Model const& model, glm::mat4 const& transform) override;
	GLuint getProgramID();
	bool loadingFailed() override;
	void release() override;
	void use() override;
protected:
	GLuint programID;
private:
	GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile);
	bool compileShader(const std::string& shaderContent, GLuint shaderResourceID);
};

#endif