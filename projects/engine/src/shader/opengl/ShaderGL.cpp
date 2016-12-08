#include <stdio.h>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <shader/opengl/ShaderGL.hpp>
#include <platform/FileSystem.hpp>
#include <util/Globals.hpp>
#include <exception/ShaderInitException.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <model/Vob.hpp>

using namespace std;
using namespace platform;
using namespace ::util;
using namespace glm;


ShaderGL::ShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile)
	: logClient(getLogServer())
{
	programID = loadShaders(vertexShaderFile, fragmentShaderFile);

	if (programID == GL_FALSE)
	{
		throw ShaderInitException("ShaderGL::ShaderGL: couldn't load shader");
	}
}

ShaderGL::ShaderGL(ShaderGL&& other) :
	logClient(other.logClient)
{
	programID = other.programID;
	other.programID = GL_FALSE;
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

GLuint ShaderGL::getProgramID() const
{
	return programID;
}

bool ShaderGL::loadingFailed() const
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

void ShaderGL::draw(const Vob& vob) const
{
}

GLuint ShaderGL::loadShaders(const string& vertexFile, const string& fragmentFile) const
{
	// Create the shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	string vertexShaderCode, fragmentShaderCode;
	GLint result = GL_FALSE;
	int infoLogLength;
	GLuint programID;

	string vertexFilePath = globals::SHADER_PATH_OPENGL + vertexFile;
	string fragmentFilePath = globals::SHADER_PATH_OPENGL + fragmentFile;

	// Read the Vertex Shader code from the file
	if (!filesystem::loadFileIntoString(vertexFilePath, &vertexShaderCode))
	{
		stringstream ss;
		ss << "Shader::loadShaders(): Couldn't initialize vertex shader!" << endl;
		ss << "vertex file: " << vertexFilePath;

		throw ShaderInitException(ss.str());
	}

	if (!filesystem::loadFileIntoString(fragmentFilePath, &fragmentShaderCode))
	{
		LOG(logClient, Error) << "Couldn't initialize fragment shader!";
		stringstream ss;
		ss << "Shader::loadShaders(): Couldn't initialize fragment shader!" << endl;
		ss << "fragment file: " << fragmentFilePath;
		throw ShaderInitException(ss.str());
	}

	if (!compileShader(vertexShaderCode.c_str(), vertexShaderID))
	{
		stringstream ss;
		ss << "Shader::loadShaders(): Couldn't compile vertex shader!" << endl;
		ss << "vertex file: " << vertexFilePath;
		throw ShaderInitException(ss.str());
	}

	if (!compileShader(fragmentShaderCode.c_str(), fragmentShaderID))
	{
		stringstream ss;
		ss << "Shader::loadShaders(): Couldn't compile fragment shader!" << endl;
		ss << "fragment file: " << fragmentFilePath;
		throw ShaderInitException(ss.str());
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


bool ShaderGL::compileShader(const string& shaderContent, GLuint shaderResourceID) const
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

	// check compilation
	glGetShaderiv(shaderResourceID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderResourceID, GL_INFO_LOG_LENGTH, &logInfoLength);

	if (logInfoLength > 0)
	{
		vector<char> shaderErrorMessage(logInfoLength + 1);
		glGetShaderInfoLog(shaderResourceID, logInfoLength, nullptr, &shaderErrorMessage[0]);
		LOG(logClient, Error) << &shaderErrorMessage[0];
	}

	if (result) return true;
	return false;
};