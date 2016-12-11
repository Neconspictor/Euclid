#ifndef ENGINE_SHADER_OPENGL_SHADERGL_HPP
#define ENGINE_SHADER_OPENGL_SHADERGL_HPP
#include <GL/glew.h>
#include <string>
#include <platform/logging/LoggingClient.hpp>

class MeshGL;
class Vob;

/**
 * Represents a shader program for an OpenGL renderer.
 */
class ShaderGL
{
public:
	/**
	* Creates a new shader program from a given vertex shader and fragment shader file.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	ShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	ShaderGL(ShaderGL&& other);
	ShaderGL(const ShaderGL& other);
	virtual ~ShaderGL();

	bool compileShader(const std::string& shaderContent, GLuint shaderResourceID) const;
	GLuint getProgramID() const;
	GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile) const;
	virtual void release();
	virtual void use();

protected:
	GLuint programID;
	platform::LoggingClient logClient;
};

#endif