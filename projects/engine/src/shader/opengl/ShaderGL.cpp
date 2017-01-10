#include <stdio.h>
#include <string>
#include <vector>
#include <shader/opengl/ShaderGL.hpp>
#include <platform/FileSystem.hpp>
#include <util/Globals.hpp>
#include <exception/ShaderInitException.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <model/Vob.hpp>
#include <renderer/opengl/RendererOpenGL.hpp>
#include <fstream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace platform;
using namespace ::util;
using namespace glm;

LoggingClient staticLogClient(getLogServer());


ShaderGL::ShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile, const string& geometryShaderFile)
	: logClient(getLogServer())
{
	programID = loadShaders(vertexShaderFile, fragmentShaderFile, geometryShaderFile);

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

void ShaderGL::initShaderFileSystem()
{
	using namespace boost::filesystem;

	if (!exists(globals::SHADER_PATH_OPENGL))
	{
		stringstream ss;
		path path(globals::SHADER_PATH_OPENGL);
		ss << "ShaderGL::initShaderFileSystem(): opengl shader folder doesn't exists: " 
			<< absolute(path).generic_string();
		throw runtime_error(ss.str());
	}

	LOG(staticLogClient, Debug) << "Test log!";

	vector<string> shaderFiles = filesystem::getFilesFromFolder(globals::SHADER_PATH_OPENGL, false);

	for (auto& file : shaderFiles)
	{
		::ifstream ifs(file);

		string content((istreambuf_iterator<char>(ifs)),
			(istreambuf_iterator<char>()));
		
		// OpenGL expects relative paths starting with a '/' 
		string glFilePath = "/" + file;
		glNamedStringARB(GL_SHADER_INCLUDE_ARB, -1, glFilePath.c_str(), -1, content.c_str());
	}
}

GLuint ShaderGL::getProgramID() const
{
	return programID;
}

void ShaderGL::release()
{
	glDeleteProgram(programID);
}


ShaderGL::~ShaderGL()
{
}

GLuint ShaderGL::loadShaders(const string& vertexFile, const string& fragmentFile, 
	const string& geometryShaderFile)
{
	// Create the shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint geometryShaderID = GL_FALSE;
	string vertexShaderCode, fragmentShaderCode, geometryShaderCode;
	GLint result = GL_FALSE;
	int infoLogLength;
	GLuint programID;

	string vertexFilePath = globals::SHADER_PATH_OPENGL + vertexFile;
	string fragmentFilePath = globals::SHADER_PATH_OPENGL + fragmentFile;
	string geometryFilePath; 
	bool useGeomtryShader = geometryShaderFile.compare("") != 0;
	if (useGeomtryShader)
	{
		geometryFilePath = globals::SHADER_PATH_OPENGL + geometryShaderFile;
		geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
	}

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
		LOG(staticLogClient, Error) << "Couldn't initialize fragment shader!";
		stringstream ss;
		ss << "Shader::loadShaders(): Couldn't initialize fragment shader!" << endl;
		ss << "fragment file: " << fragmentFilePath;
		throw ShaderInitException(ss.str());
	}

	if (useGeomtryShader)
	{
		if (!filesystem::loadFileIntoString(geometryFilePath, &geometryShaderCode))
		{
			LOG(staticLogClient, Error) << "Couldn't initialize geometry shader!";
			stringstream ss;
			ss << "Shader::loadShaders(): Couldn't initialize geometry shader!" << endl;
			ss << "geometry shader file: " << geometryFilePath;
			throw ShaderInitException(ss.str());
		}
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

	if (useGeomtryShader)
	{
		if (!compileShader(geometryShaderCode.c_str(), geometryShaderID))
		{
			stringstream ss;
			ss << "Shader::loadShaders(): Couldn't compile geometry shader!" << endl;
			ss << "geometry file: " << geometryFilePath;
			throw ShaderInitException(ss.str());
		}
	}

	// link the program
	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	if (useGeomtryShader) 
		glAttachShader(programID, geometryShaderID);

	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (result == GL_FALSE)
	{
		if (infoLogLength > 0) {
			vector<char> ProgramErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(programID, infoLogLength, nullptr, &ProgramErrorMessage[0]);
			LOG(staticLogClient, Error) << &ProgramErrorMessage[0];
		}
		throw ShaderInitException("Error: Shader::loadShaders(): Couldn't create shader program!");
	}

	// release not needed memory
	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);
	if (useGeomtryShader)
	{
		glDetachShader(programID, geometryShaderID);
		glDeleteShader(geometryShaderID);
	}

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
		LOG(staticLogClient, Error) << "shaderContent is suppossed to be no null-string!";
		return GL_FALSE;
	}

	// compile...
	const char* rawCode = shaderContent.c_str();
	const GLchar* const test  = "/shaders/opengl";
	glShaderSource(shaderResourceID, 1, &rawCode, nullptr);

	RendererOpenGL::checkGLErrors("ShaderGL.cpp");

	glCompileShaderIncludeARB(shaderResourceID, 1, &test, nullptr);

	RendererOpenGL::checkGLErrors("ShaderGL.cpp2");

	//glCompileShader(shaderResourceID);

	// check compilation
	glGetShaderiv(shaderResourceID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderResourceID, GL_INFO_LOG_LENGTH, &logInfoLength);

	if (logInfoLength > 0)
	{
		vector<char> shaderErrorMessage(logInfoLength + 1);
		glGetShaderInfoLog(shaderResourceID, logInfoLength, nullptr, &shaderErrorMessage[0]);
		LOG(staticLogClient, Error) << &shaderErrorMessage[0];
	}

	if (result) return true;
	return false;
};