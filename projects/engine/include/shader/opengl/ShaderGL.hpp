#pragma once
#include <string>
#include <platform/logging/LoggingClient.hpp>
#include <glad/glad.h>
#include <shader/Shader.hpp>
#include <string>

class MeshGL;
class Vob;

class ShaderAttributeGL : public ShaderAttribute
{
public:
	ShaderAttributeGL();
	ShaderAttributeGL(ShaderAttributeType type, void* data, std::string uniformName, bool active = false);
	virtual ~ShaderAttributeGL() override;

	const std::string& getName() const;

	void setData(void* data);
	void setName(std::string name);
	void setType(ShaderAttributeType type);

	using ShaderVec = std::vector<ShaderAttributeGL>;
	static ShaderAttributeGL* search(const ShaderVec& vec, std::string uniformName);

protected:
	std::string uniformName;
};

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
	ShaderGL(ShaderConfig* config, const std::string& vertexShaderFile, const std::string& fragmentShaderFile,
		const std::string& geometryShaderFile = "");
	ShaderGL(ShaderGL&& other);
	ShaderGL(const ShaderGL& other);
	virtual ~ShaderGL();

	static bool compileShader(const std::string& shaderContent, GLuint shaderResourceID);

	void draw(Mesh const& mesh) override;

	void drawInstanced(Mesh const& mesh, unsigned amount) override;

	GLuint getProgramID() const;
	
	static GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile, 
		const std::string& geometryShaderFile = "");
	
	virtual void release();
	
	virtual void use();

	static void initShaderFileSystem();

protected:
	ShaderConfig* config;
	GLuint programID;
	GLuint instancedProgramID;
	platform::LoggingClient logClient;
	GLint textureCounter;

	void setAttribute(GLuint program, const ShaderAttributeGL& attribute);
};