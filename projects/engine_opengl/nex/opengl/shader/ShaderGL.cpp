#include <string>
#include <vector>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/FileSystem.hpp>
#include <nex/util/Globals.hpp>
#include <nex/exception/ShaderInitException.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <fstream>
#include <boost/filesystem.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>
#include <regex>
#include "nex/opengl/mesh/MeshGL.hpp"

using namespace nex;
using namespace ::util;
using namespace glm;

Logger staticLogClient("ShaderGL-static");

/**
* Maps shader enumerations to a string representation.
*/
const static nex::util::EnumString<Shaders> shaderEnumConversion[] = {
	{ Shaders::BlinnPhongTex, "BLINN_PHONG_TEX" },
{ Shaders::Pbr, "PBR" },
{ Shaders::Pbr_Deferred_Geometry, "PBR_DEFERRED_GEOMETRY" },
{ Shaders::Pbr_Deferred_Lighting, "PBR_DEFERRED_LIGHTING" },
{ Shaders::Pbr_Convolution, "PBR_CONVOLUTION" },
{ Shaders::Pbr_Prefilter, "PBR_PREFILTER" },
{ Shaders::Pbr_BrdfPrecompute, "PBR_BRDF_PRECOMPUTE" },
{ Shaders::CubeDepthMap, "CUBE_DEPTH_MAP" },
{ Shaders::DepthMap, "DEPTH_MAP" },
{ Shaders::GaussianBlurHorizontal, "GAUSSIAN_BLUR_HORIZONTAL" },
{ Shaders::GaussianBlurVertical, "GAUSSIAN_BLUR_VERTICAL" },
{ Shaders::Normals, "NORMALS" },
{ Shaders::Shadow, "SHADOW" },
{ Shaders::ShadowPoint, "SHADOW_POINT" },
{ Shaders::SimpleColor, "SIMPLE_COLOR" },
{ Shaders::SimpleExtrude, "SIMPLE_EXTRUDE" },
{ Shaders::Screen, "SCREEN" },
{ Shaders::SkyBox, "SKY_BOX" },
{ Shaders::SkyBoxEquirectangular, "SKY_BOX_EQUIRECTANGULAR" },
{ Shaders::SkyBoxPanorama, "SKY_BOX_PANORAMA" },
{ Shaders::VarianceShadow, "VARIANCE_DEPTH_MAP" },
{ Shaders::VarianceShadow, "VARIANCE_SHADOW" }
};


Shaders stringToShaderEnum(const std::string& str)
{
	return stringToEnum(str, shaderEnumConversion);
}

std::ostream& operator<<(std::ostream& os, Shaders shader)
{
	os << enumToString(shader, shaderEnumConversion);
	return os;
}

ShaderGL::ShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile,
	const std::string& geometryShaderFile, const std::string& instancedVertexShaderFile)
{
	programID = loadShaders(vertexShaderFile, fragmentShaderFile, geometryShaderFile);
	if (programID == GL_FALSE)
	{
		throw_with_trace(ShaderInitException("ShaderGL::ShaderGL: couldn't load shader"));
	}

	/*if (instancedVertexShaderFile != "")
	{
		instancedProgramID = loadShaders(instancedVertexShaderFile, fragmentShaderFile, geometryShaderFile);
		if (instancedProgramID == GL_FALSE)
		{
			throw_with_trace(ShaderInitException("ShaderGL::ShaderGL: couldn't load instanced shader"));
		}
	}*/
}

ShaderGL::ShaderGL(ShaderGL&& other) :
    programID(other.programID)
{
	other.programID = GL_FALSE;
}

ShaderGL::~ShaderGL() {}

void ShaderGL::initShaderFileSystem()
{
	using namespace boost::filesystem;

	if (!exists(Globals::getOpenGLShaderPath()))
	{
		std::stringstream ss;
		path path(Globals::getOpenGLShaderPath());
		ss << "ShaderGL::initShaderFileSystem(): opengl shader folder doesn't exists: " 
			<< absolute(path).generic_string();
		throw_with_trace(std::runtime_error(ss.str()));
	}

	std::vector<std::string> shaderFiles = filesystem::getFilesFromFolder(Globals::getOpenGLShaderPath(), false);

	for (auto& file : shaderFiles)
	{
		std::ifstream ifs(file);

		std::string content((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
		
		// OpenGL expects relative paths starting with a '/' 
		std::string glFilePath = "/" + file;
		//glNamedStringARB(GL_SHADER_INCLUDE_ARB, -1, glFilePath.c_str(), -1, content.c_str()); // TODO ARB_shading_language is only supported
		// on NVIDIA GPUs => find a GPU independent solution; For now, includes are deactivated in all shaders
	}
}


void ShaderGL::setInt(GLuint uniformID, int data)
{
	GLCall(glUniform1i(uniformID, data));
}

void ShaderGL::setFloat(GLuint uniformID, float data)
{
	GLCall(glUniform1f(uniformID, data));
}

void ShaderGL::setVec2(GLuint uniformID, const glm::vec2& data)
{
	GLCall(glUniform2f(uniformID, data.x, data.y));
}

void ShaderGL::setVec3(GLuint uniformID, const glm::vec3& data)
{
	GLCall(glUniform3f(uniformID, data.x, data.y, data.z));
}

void ShaderGL::setVec4(GLuint uniformID, const glm::vec4& data)
{
	GLCall(glUniform4f(uniformID, data.x, data.y, data.z, data.w));
}

void ShaderGL::setMat3(GLuint uniformID, const glm::mat3& data)
{
	GLCall(glUniformMatrix3fv(uniformID, 1, GL_FALSE, value_ptr(data)));
}

void ShaderGL::setMat4(GLuint uniformID, const glm::mat4& data)
{
	GLCall(glUniformMatrix4fv(uniformID, 1, GL_FALSE, value_ptr(data)));
}

void ShaderGL::setTexture2D(GLuint uniformID, const TextureGL* data, unsigned int textureSlot)
{
	assert(textureSlot < 32); // OpenGL allows up to 32 textures
	GLCall(glActiveTexture(textureSlot + GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_2D, data->getTexture()));
	GLCall(glUniform1i(uniformID, textureSlot));
}

void ShaderGL::setTexture2DArray(GLuint uniformID, const TextureGL* data, unsigned int textureSlot)
{
	assert(textureSlot < 32); // OpenGL allows up to 32 textures
	GLCall(glActiveTexture(textureSlot + GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_2D_ARRAY, data->getTexture()));
	GLCall(glUniform1i(uniformID, textureSlot));
}

void ShaderGL::setCubeMap(GLuint uniformID, const CubeMapGL* data, unsigned int textureSlot)
{
	assert(textureSlot < 32); // OpenGL allows up to 32 textures
	GLCall(glActiveTexture(textureSlot + GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, data->getTexture()));
	GLCall(glUniform1i(uniformID, textureSlot));
}

void ShaderGL::setCubeMapArray(GLuint uniformID, const CubeMapGL* data, unsigned textureSlot)
{
	assert(textureSlot < 32); // OpenGL allows up to 32 textures
	GLCall(glActiveTexture(textureSlot + GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, data->getTexture()));
	GLCall(glUniform1i(uniformID, textureSlot));
}

GLuint ShaderGL::getProgramID() const
{
	return programID;
}

unsigned ShaderGL::getUniformLocation(const char* name)
{
	auto loc = glGetUniformLocation(programID, name);

	if (loc < 0)
	{
		static Logger logger("ShaderGL::getUniformLocation");
		logger(Debug) << "Uniform '" << name << "' doesn't exist in shader '" << mDebugName << "'";
	}

	return loc;
}

void ShaderGL::release()
{
	glDeleteProgram(programID);
}

void ShaderGL::setDebugName(const char* name)
{
	mDebugName = name;
}

void ShaderGL::unbind() const
{
	GLCall(glUseProgram(0));
}

GLuint ShaderGL::loadShaders(const std::string& vertexFile, const std::string& fragmentFile,
	const std::string& geometryShaderFile)
{
	// Create the shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint geometryShaderID = GL_FALSE;
	std::string vertexShaderCode, fragmentShaderCode, geometryShaderCode;
	GLint result = GL_FALSE;
	int infoLogLength;
	GLuint programID;

	std::string vertexFilePath = Globals::getOpenGLShaderPath() + vertexFile;
	std::string fragmentFilePath = Globals::getOpenGLShaderPath() + fragmentFile;
	std::string geometryFilePath;
	bool useGeomtryShader = geometryShaderFile.compare("") != 0;
	if (useGeomtryShader)
	{
		geometryFilePath = Globals::getOpenGLShaderPath() + geometryShaderFile;
		geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
	}

	// Read the Vertex Shader code from file
	try
	{
		vertexShaderCode = loadShaderComponent(vertexFilePath);
	} catch(const ShaderInitException e)
	{
		LOG(staticLogClient, Error) << e.what();
		throw_with_trace(ShaderInitException("Couldn't initialize vertex shader: " + vertexFilePath));
	}

	// Read the Fragment Shader code from file
	try
	{
		fragmentShaderCode = loadShaderComponent(fragmentFilePath);
	}
	catch (const ShaderInitException e)
	{
		LOG(staticLogClient, Error) << e.what();
		throw_with_trace(ShaderInitException("Couldn't initialize fragment shader: " + fragmentFilePath));
	}


	if (useGeomtryShader)
	{
		// Read the geometry Shader code from file
		try
		{
			geometryShaderCode = loadShaderComponent(geometryFilePath);
		}
		catch (const ShaderInitException e)
		{
			LOG(staticLogClient, Error) << e.what();
			throw_with_trace(ShaderInitException("Couldn't initialize geometry shader: " + geometryFilePath));
		}
	}

	if (!compileShaderComponent(vertexShaderCode.c_str(), vertexShaderID))
	{
		std::stringstream ss;
		ss << "Shader::loadShaders(): Couldn't compile vertex shader!" << std::endl;
		ss << "vertex file: " << vertexFilePath;
		throw_with_trace(ShaderInitException(ss.str()));
	}

	if (!compileShaderComponent(fragmentShaderCode.c_str(), fragmentShaderID))
	{
		std::stringstream ss;
		ss << "Shader::loadShaders(): Couldn't compile fragment shader!" << std::endl;
		ss << "fragment file: " << fragmentFilePath;
		throw_with_trace(ShaderInitException(ss.str()));
	}

	if (useGeomtryShader)
	{
		if (!compileShaderComponent(geometryShaderCode.c_str(), geometryShaderID))
		{
			std::stringstream ss;
			ss << "Shader::loadShaders(): Couldn't compile geometry shader!" << std::endl;
			ss << "geometry file: " << geometryFilePath;
			throw_with_trace(ShaderInitException(ss.str()));
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
			std::vector<char> ProgramErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(programID, infoLogLength, nullptr, &ProgramErrorMessage[0]);
			LOG(staticLogClient, Error) << &ProgramErrorMessage[0];
		}
		throw_with_trace(ShaderInitException("Error: Shader::loadShaders(): Couldn't create shader program!"));
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


bool ShaderGL::compileShaderComponent(const std::string& shaderContent, GLuint shaderResourceID)
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
	glShaderSource(shaderResourceID, 1, &rawCode, nullptr);

	glCompileShader(shaderResourceID);

	RendererOpenGL::checkGLErrors("ShaderGL.cpp");

	// check compilation
	glGetShaderiv(shaderResourceID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderResourceID, GL_INFO_LOG_LENGTH, &logInfoLength);

	if (logInfoLength > 0)
	{
		std::vector<char> shaderErrorMessage(logInfoLength + 1);
		glGetShaderInfoLog(shaderResourceID, logInfoLength, nullptr, &shaderErrorMessage[0]);
		LOG(staticLogClient, Error) << &shaderErrorMessage[0];
	}

	if (result) return true;
	return false;
}

void ShaderGL::bind() const
{
	GLCall(glUseProgram(programID));
}


bool ShaderGL::extractIncludeStatement(const std::string& line, std::string* result)
{
	std::regex pattern1("#include.*<.*>.*");
	std::regex pattern2("#include.*\\\".*\\\".*");

	char separatorToken1;
	char separatorToken2;

	if (std::regex_match(line.begin(), line.end(), pattern1))
	{
		separatorToken1 = '<';
		separatorToken2 = '>';
	}
	else if (std::regex_match(line.begin(), line.end(), pattern2))
	{
		separatorToken1 = separatorToken2 = '\"';
	}
	else
	{
		return false;
	}

	auto pos = line.find_first_of(separatorToken1);
	auto start = line.begin() + pos + 1;
	auto end = start;
	for (; *end != separatorToken2; ++end) {}

	*result = std::string(start, end);
	nex::util::Trim::trim(*result);

	return true;
}

std::string ShaderGL::loadShaderComponent(const std::string& shaderFile, bool writeUnfoldedResult, std::vector<std::string> defines)
{
	std::string content;
	if (!nex::filesystem::loadFileIntoString(shaderFile, &content))
	{
		std::stringstream ss;
		ss << "Shader::loadShaderComponent(): Couldn't load shader file" << std::endl;
		ss << "Shader file: " << shaderFile;

		throw_with_trace(ShaderInitException(ss.str()));
	}

	// preprocess throws ShaderInitException if it cannot preprocess the content
	std::vector<std::string> lines = preprocess(std::move(content), defines);

	if (writeUnfoldedResult)
		writeUnfoldedShaderContentToFile(shaderFile, lines);


	// Build result string
	std::stringstream ss;
	for (auto& line : lines)
		ss << line << std::endl;
	
	return ss.str();
}

std::vector<std::string> ShaderGL::preprocess(std::string& shaderContent, const std::vector<std::string>& defines)
{
	using namespace nex::util;
	using namespace std;

	removeComments(shaderContent);
	auto lines = tokenize(shaderContent, "\n");

	auto eraseIt = remove_if(lines.begin(), lines.end(), [&](string& line)
	{
		Trim::trim(line);
		return line.empty();
	});

	lines.erase(eraseIt, lines.end());

	// We removed comments, trimmed the lines and removed empty lines
	// Thus the first line in a valid glsl shader file defines the glsl version
	// So we add the defines just after the first line

	if (lines.size() >= 1)
	{
		for (int i = 0; i < defines.size(); ++i)
		{
			lines.insert(lines.begin() + 1 + i, defines[i]);
		}
	}

	for (auto nextIt = lines.begin(); nextIt != lines.end(); )
	{
		std::string includeFile;

		auto currentIt = nextIt;
		++nextIt;

		if (extractIncludeStatement(*currentIt, &includeFile))
		{
			std::string content;
			if (!::filesystem::loadFileIntoString(includeFile, &content))
			{
				std::stringstream ss;
				ss << "Shader::preprocess(): Couldn't load include file!" << std::endl;
				ss << "Include file: " << includeFile;

				throw_with_trace(ShaderInitException(ss.str()));
			}
			std::vector<std::string> includeLines = preprocess(content, {});

			auto insertionSize = includeLines.size();
			currentIt = lines.insert(currentIt, includeLines.begin(), includeLines.end());
			currentIt += insertionSize;

			// remove the current line and let it point to the next line
			nextIt = lines.erase(currentIt);
		}
	}

	return lines;
}

void ShaderGL::writeUnfoldedShaderContentToFile(const std::string& shaderSourceFile, const std::vector<std::string>& lines)
{
	nex::filesystem::writeToFile(shaderSourceFile + ".unfolded", lines, std::ostream::trunc);
}

/*
void ShaderGL::beforeDrawing(const MeshGL& mesh)
{
	if (config != nullptr) {
		config->beforeDrawing(mesh);

		glUseProgram(programID);

		config->update(mesh, data);

		auto attributes = reinterpret_cast<const ShaderAttributeGL*> (config->getAttributeList());
		for (int i = 0; i < config->getNumberOfAttributes(); ++i)
		{
			setAttribute(programID, attributes[i]);
		}
}
}*/