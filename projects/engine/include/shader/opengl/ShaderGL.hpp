#pragma once
#include <string>
#include <platform/logging/LoggingClient.hpp>
#include <glad/glad.h>

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
	ShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile,
		const std::string& geometryShaderFile = "");
	ShaderGL(ShaderGL&& other);
	ShaderGL(const ShaderGL& other);
	virtual ~ShaderGL();

	bool compileShader(const std::string& shaderContent, GLuint shaderResourceID) const;
	GLuint getProgramID() const;
	GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile, 
		const std::string& geometryShaderFile) const;
	virtual void release();
	virtual void use();

	static void initShaderFileSystem();

protected:
	GLuint programID;
	platform::LoggingClient logClient;
};