#ifndef ENGINE_SHADER_OPENGL_SHADERGL_HPP
#define ENGINE_SHADER_OPENGL_SHADERGL_HPP
#include <shader/Shader.hpp>
#include <GL/glew.h>
#include <string>
#include <platform/logging/LoggingClient.hpp>

class MeshGL;
class Model;

/**
 * Represents a shader program for an OpenGL renderer.
 */
class ShaderGL : public Shader
{
public:
	/**
	* Creates a new shader program from a given vertex shader and fragment shader file.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	ShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	ShaderGL(ShaderGL&& other);
	ShaderGL(const ShaderGL& other);
	virtual ~ShaderGL() override;

	bool compileShader(const std::string& shaderContent, GLuint shaderResourceID);
	void draw(Model const& model, glm::mat4 const& projection, glm::mat4 const& view) override;
	GLuint getProgramID();
	bool loadingFailed() override;
	GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile);
	void release() override;
	void use() override;

protected:
	GLuint programID;
	platform::LoggingClient logClient;

	static MeshGL* getFromModel(Model const& model);
};

#endif