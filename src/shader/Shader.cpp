#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>

#include "shader/shader.hpp"
#include <util/Util.hpp>

using namespace std;
using namespace util;



Shader::Shader(const string& vertexShaderFile, const string& fragmentShaderFile)
{
	programID = loadShaders(vertexShaderFile, fragmentShaderFile);

	if (programID == GL_FALSE)
	{
		cerr << "Shader::Error: Couldn't construct shader properly!" << endl;
	}
}

void Shader::use()
{
	glUseProgram(this->programID);
}

GLuint Shader::getProgramID()
{
	return programID;
}

bool Shader::loadingFailed()
{
	return this->programID == GL_FALSE;
}

void Shader::release()
{
	glDeleteProgram(programID);
}

void Shader::draw(Model const& model, glm::mat4 const& transform)
{
	use();
	glBindVertexArray(model.getVertexArrayObject());
	glDrawArrays(GL_TRIANGLES, 0, model.getVertexCount());
	glBindVertexArray(0);
}

GLuint Shader::loadShaders(const string& vertexFile, const string& fragmentFile)
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
	if (!loadShaderFromFile(vertexFilePath, &vertexShaderCode))
	{
		cerr << "Error: Shader::loadShaders(): Couldn't initialize vertex shader!" << endl;
		return GL_FALSE;
	}

	if (!loadShaderFromFile(fragmentFilePath, &fragmentShaderCode))
	{
		cerr << "Error: Shader::loadShaders(): Couldn't initialize fragment shader!" << endl;
		return GL_FALSE;
	}

	if (!compileShader(vertexShaderCode.c_str(), vertexShaderID))
	{
		cerr << "Error: Shader::loadShaders(): Couldn't compile vertex shader!" << endl;
		return GL_FALSE;
	}

	if (!compileShader(fragmentShaderCode.c_str(), fragmentShaderID))
	{
		cerr << "Error: Shader::loadShaders(): Couldn't compile fragment shader!" << endl;
		return GL_FALSE;
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

		cerr << "Error: Shader::loadShaders(): Couldn't create shader program!" << endl;

		if (infoLogLength > 0) {
			vector<char> ProgramErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(programID, infoLogLength, nullptr, &ProgramErrorMessage[0]);
			cerr << "Error: Shader::loadShaders(): " << &ProgramErrorMessage[0] << endl;
		}

		return GL_FALSE;
	}

	// release not needed memory
	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}

bool Shader::loadShaderFromFile(const string& file, string* shaderContent)
{
	ifstream shaderStreamFile;
	bool loadingWasSuccessful = true;

	// ensure ifstream can throw exceptions!
	shaderStreamFile.exceptions(ifstream::failbit | ifstream::badbit);

	try
	{
		shaderStreamFile.open(file);
		stringstream shaderStream;

		// read file content to stream
		shaderStream << shaderStreamFile.rdbuf();
		*shaderContent = shaderStream.str();
	}
	catch (ifstream::failure e)
	{
		if (shaderStreamFile.fail())
		{
			cerr << "Error: Shader::loadShaderFromFile(): Couldn't opened file: "
				<< file << endl;
		}

		if (shaderStreamFile.bad())
		{
			cerr << "Error: Shader::loadShaderFromFile() : Couldn't read file properly." << endl;
		}
		loadingWasSuccessful = false;
		*shaderContent = "";
	}

	//clear exceptions as close shouldn't throw any exceptions!
	shaderStreamFile.exceptions(0);
	shaderStreamFile.close();

	return loadingWasSuccessful;
}

bool Shader::compileShader(const string& shaderContent, GLuint shaderResourceID)
{
	int result = 0;
	GLint logInfoLength;

	if (!shaderContent.size())
	{
		cerr << "Error: Shader::compileShader: shaderContent is suppossed to be no null-string!" << endl;
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
		cerr << "Error: Shader::compileShader: " << &shaderErrorMessage[0] << endl;
	}

	return result;
};