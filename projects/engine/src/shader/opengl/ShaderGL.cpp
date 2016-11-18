#include <stdio.h>
#include <string>
#include <vector>

#include <GL/glew.h>

#include <shader/opengl/ShaderGL.hpp>
#include <platform/FileSystem.hpp>
#include <util/GlobalPaths.hpp>
#include <exception/ShaderInitException.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;
using namespace platform;
using namespace ::util;


ShaderGL::ShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile)
	: logClient(getLogServer())
{
	programID = loadShaders(vertexShaderFile, fragmentShaderFile);

	if (programID == GL_FALSE)
	{
		throw ShaderInitException("ShaderGL::ShaderGL: couldn't load shader");
	}
}

ShaderGL::ShaderGL(const ShaderGL& other) : 
	logClient(other.logClient)
{
	programID = other.programID;
}

void ShaderGL::use()
{
	glUseProgram(this->programID);
}

GLuint ShaderGL::getProgramID()
{
	return programID;
}

bool ShaderGL::loadingFailed()
{
	return this->programID == GL_FALSE;
}

void ShaderGL::release()
{
	glDeleteProgram(programID);
}

ShaderGL::~ShaderGL()
{
}

void ShaderGL::draw(Model const& model, glm::mat4 const& transform)
{
	use();
	glBindVertexArray(model.getVertexArrayObject());
	glDrawArrays(GL_TRIANGLES, 0, model.getVertexCount());
	glBindVertexArray(0);
}

GLuint ShaderGL::loadShaders(const string& vertexFile, const string& fragmentFile)
{
	// Create the shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	string vertexShaderCode, fragmentShaderCode;
	GLint result = GL_FALSE;
	int infoLogLength;
	GLuint programID;

	string vertexFilePath = global_path::SHADER_PATH + vertexFile;
	string fragmentFilePath = global_path::SHADER_PATH + fragmentFile;

	// Read the Vertex Shader code from the file
	if (!filesystem::loadFileIntoString(vertexFilePath, &vertexShaderCode))
	{
		throw ShaderInitException("Shader::loadShaders(): Couldn't initialize vertex shader!");
	}

	if (!filesystem::loadFileIntoString(fragmentFilePath, &fragmentShaderCode))
	{
		LOG(logClient, Error) << "Couldn't initialize fragment shader!";
		throw ShaderInitException("Shader::loadShaders(): Couldn't initialize fragment shader!");
	}

	if (!compileShader(vertexShaderCode.c_str(), vertexShaderID))
	{
		throw ShaderInitException("Shader::loadShaders(): Couldn't compile vertex shader!");
	}

	if (!compileShader(fragmentShaderCode.c_str(), fragmentShaderID))
	{
		throw ShaderInitException("Shader::loadShaders(): Couldn't compile fragment shader!");
	}

	// link the program
	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (result == GL_FALSE)
	{
		if (infoLogLength > 0) {
			vector<char> ProgramErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(programID, infoLogLength, nullptr, &ProgramErrorMessage[0]);
			LOG(logClient, Error) << &ProgramErrorMessage[0];
		}
		throw ShaderInitException("Error: Shader::loadShaders(): Couldn't create shader program!");
	}

	// release not needed memory
	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}


bool ShaderGL::compileShader(const string& shaderContent, GLuint shaderResourceID)
{
	int result = 0;
	GLint logInfoLength;

	if (!shaderContent.size())
	{
		LOG(logClient, Error) << "shaderContent is suppossed to be no null-string!";
		return GL_FALSE;
	}

	// compile...
	const char* rawCode = shaderContent.c_str();
	glShaderSource(shaderResourceID, 1, &rawCode, nullptr);
	glCompileShader(shaderResourceID);

	//check compilation
	glGetShaderiv(shaderResourceID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderResourceID, GL_INFO_LOG_LENGTH, &logInfoLength);

	if (logInfoLength > 0)
	{
		vector<char> shaderErrorMessage(logInfoLength + 1);
		glGetShaderInfoLog(shaderResourceID, logInfoLength, nullptr, &shaderErrorMessage[0]);
		LOG(logClient, Error) << &shaderErrorMessage[0];
	}

	return result;
};