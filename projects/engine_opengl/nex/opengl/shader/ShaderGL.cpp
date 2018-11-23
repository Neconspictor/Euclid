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
#include "nex/util/Memory.hpp"
#include <nex/opengl/material/Material.hpp>
#include "nex/shader_generator/SourceFileConsumer.hpp"

using namespace nex;
using namespace ::util;
using namespace glm;

Logger staticLogClient("ShaderGL-static");

/**
* Maps shader enumerations to a string representation.
*/
const static nex::util::EnumString<ShaderType> shaderEnumConversion[] = {
	{ ShaderType::BlinnPhongTex, "BLINN_PHONG_TEX" },
{ ShaderType::Pbr, "PBR" },
{ ShaderType::Pbr_Deferred_Geometry, "PBR_DEFERRED_GEOMETRY" },
{ ShaderType::Pbr_Deferred_Lighting, "PBR_DEFERRED_LIGHTING" },
{ ShaderType::Pbr_Convolution, "PBR_CONVOLUTION" },
{ ShaderType::Pbr_Prefilter, "PBR_PREFILTER" },
{ ShaderType::Pbr_BrdfPrecompute, "PBR_BRDF_PRECOMPUTE" },
{ ShaderType::CubeDepthMap, "CUBE_DEPTH_MAP" },
{ ShaderType::DepthMap, "DEPTH_MAP" },
{ ShaderType::GaussianBlurHorizontal, "GAUSSIAN_BLUR_HORIZONTAL" },
{ ShaderType::GaussianBlurVertical, "GAUSSIAN_BLUR_VERTICAL" },
{ ShaderType::Normals, "NORMALS" },
{ ShaderType::Shadow, "SHADOW" },
{ ShaderType::ShadowPoint, "SHADOW_POINT" },
{ ShaderType::SimpleColor, "SIMPLE_COLOR" },
{ ShaderType::SimpleExtrude, "SIMPLE_EXTRUDE" },
{ ShaderType::Screen, "SCREEN" },
{ ShaderType::SkyBox, "SKY_BOX" },
{ ShaderType::SkyBoxEquirectangular, "SKY_BOX_EQUIRECTANGULAR" },
{ ShaderType::SkyBoxPanorama, "SKY_BOX_PANORAMA" },
{ ShaderType::VarianceShadow, "VARIANCE_DEPTH_MAP" },
{ ShaderType::VarianceShadow, "VARIANCE_SHADOW" }
};


ShaderType stringToShaderEnum(const std::string& str)
{
	return stringToEnum(str, shaderEnumConversion);
}

std::ostream& operator<<(std::ostream& os, ShaderType shader)
{
	os << enumToString(shader, shaderEnumConversion);
	return os;
}

ShaderProgramGL::ShaderProgramGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile,
	const std::string& geometryShaderFile, const std::string& instancedVertexShaderFile) : mIsBound(false)
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

ShaderProgramGL::ShaderProgramGL(ShaderProgramGL&& other) :
	programID(other.programID), mIsBound(other.mIsBound), mDebugName(std::move(other.mDebugName))
{
	other.programID = GL_FALSE;
}


void ShaderProgramGL::setInt(GLint uniformID, int data)
{
	assert(mIsBound);
	if (uniformID < 0) return;
	glUniform1i(uniformID, data);
}

void ShaderProgramGL::setFloat(GLint uniformID, float data)
{
	assert(mIsBound);
	if (uniformID < 0) return;
	glUniform1f(uniformID, data);
}

void ShaderProgramGL::setVec2(GLint uniformID, const glm::vec2& data)
{
	assert(mIsBound);
	if (uniformID < 0) return;
	glUniform2f(uniformID, data.x, data.y);
}

void ShaderProgramGL::setVec3(GLint uniformID, const glm::vec3& data)
{
	ASSERT(mIsBound);
	if (uniformID < 0) return;

	//GLClearError();
	glUniform3f(uniformID, data.x, data.y, data.z);

	/*if (!GLLogCall())
	{
		
		nex::Logger("ShaderProgramGL")(nex::Error) << "Something went wrong!";
	}*/
}

void ShaderProgramGL::setVec4(GLint uniformID, const glm::vec4& data)
{
	assert(mIsBound);
	if (uniformID < 0) return;

	glUniform4f(uniformID, data.x, data.y, data.z, data.w);
}

void ShaderProgramGL::setMat3(GLint uniformID, const glm::mat3& data)
{
	assert(mIsBound);
	if (uniformID < 0) return;
	glUniformMatrix3fv(uniformID, 1, GL_FALSE, value_ptr(data));
}

void ShaderProgramGL::setMat4(GLint uniformID, const glm::mat4& data)
{
	assert(mIsBound);
	if (uniformID < 0) return;
	glUniformMatrix4fv(uniformID, 1, GL_FALSE, value_ptr(data));
}

void ShaderProgramGL::setTexture(GLint uniformID, const TextureGL* data, unsigned textureSlot)
{
	//assert(mIsBound);
	ASSERT(mIsBound);
	ASSERT(isValid(textureSlot));
	if (uniformID < 0) return;

	glBindTextureUnit(textureSlot, data->getTexture());
	glUniform1i(uniformID, textureSlot);
}


/*

void ShaderGL::setTexture2D(GLuint uniformID, const TextureGL* data, unsigned int textureSlot)
{
	assert(isValid(textureSlot));
	
	GLCall(glBindTextureUnit(textureSlot, data->getTexture()));
	
	//GLCall(glActiveTexture(textureSlot + GL_TEXTURE0));
	//GLCall(glBindTexture(GL_TEXTURE_2D, data->getTexture()));
	GLCall(glUniform1i(uniformID, textureSlot));
}

void ShaderGL::setTexture2DArray(GLuint uniformID, const TextureGL* data, unsigned int textureSlot)
{
	assert(isValid(textureSlot));

	GLCall(glBindTextureUnit(textureSlot, data->getTexture()));
	
	//GLCall(glActiveTexture(textureSlot + GL_TEXTURE0));
	//GLCall(glBindTexture(GL_TEXTURE_2D_ARRAY, data->getTexture()));
	GLCall(glUniform1i(uniformID, textureSlot));
}

void ShaderGL::setCubeMap(GLuint uniformID, const CubeMapGL* data, unsigned int textureSlot)
{
	assert(isValid(textureSlot));

	GLCall(glBindTextureUnit(textureSlot, data->getTexture()));

	//GLCall(glActiveTexture(textureSlot + GL_TEXTURE0));
	//GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, data->getTexture()));
	GLCall(glUniform1i(uniformID, textureSlot));
}

void ShaderGL::setCubeMapArray(GLuint uniformID, const CubeMapGL* data, unsigned textureSlot)
{
	assert(isValid(textureSlot));

	GLCall(glBindTextureUnit(textureSlot, data->getTexture()));
	
	//GLCall(glActiveTexture(textureSlot + GL_TEXTURE0));
	//GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, data->getTexture()));
	GLCall(glUniform1i(uniformID, textureSlot));
}
*/

GLuint ShaderProgramGL::getProgramID() const
{
	return programID;
}

unsigned ShaderProgramGL::getUniformLocation(const char* name)
{
	auto loc = glGetUniformLocation(programID, name);

	if (loc < 0)
	{
		static Logger logger("ShaderGL::getUniformLocation");
		logger(Debug) << "Uniform '" << name << "' doesn't exist in shader '" << mDebugName << "'";
	}

	return loc;
}

void ShaderProgramGL::release()
{
	glDeleteProgram(programID);
}

void ShaderProgramGL::setDebugName(const char* name)
{
	mDebugName = name;
}

void ShaderProgramGL::unbind()
{
	GLCall(glUseProgram(0));
	mIsBound = false;
}

GLuint ShaderProgramGL::loadShaders(const std::string& vertexFile, const std::string& fragmentFile,
	const std::string& geometryShaderFile)
{	

	bool useGeomtryShader = geometryShaderFile.compare("") != 0;

	FileDesc vertexFileDesc;
	FileDesc fragmentFileDesc;
	FileDesc geometryFileDesc;

	ShaderSourceFileGenerator* generator = getSourceFileGenerator();


	// Read the Vertex Shader code from file
	try
	{
		vertexFileDesc = generator->generate(vertexFile);
	} catch(const ParseException& e)
	{
		LOG(staticLogClient, Error) << e.what();
		throw_with_trace(ShaderInitException("Couldn't initialize vertex shader: " + vertexFile));
	}

	// Read the Fragment Shader code from file
	try
	{
		fragmentFileDesc = generator->generate(fragmentFile);
	}
	catch (const ShaderInitException e)
	{
		LOG(staticLogClient, Error) << e.what();
		throw_with_trace(ShaderInitException("Couldn't initialize fragment shader: " + fragmentFile));
	}


	if (useGeomtryShader)
	{
		// Read the geometry Shader code from file
		try
		{
			geometryFileDesc = generator->generate(geometryShaderFile);
		}
		catch (const ShaderInitException e)
		{
			LOG(staticLogClient, Error) << e.what();
			throw_with_trace(ShaderInitException("Couldn't initialize geometry shader: " + geometryShaderFile));
		}
	}


	// Create the shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint geometryShaderID = GL_FALSE;
	
	if (useGeomtryShader)
	{
		geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
	}
	
	std::string vertexShaderCode, fragmentShaderCode, geometryShaderCode;
	GLint result = GL_FALSE;
	int infoLogLength;
	GLuint programID;


	if (!compileShaderComponent(vertexShaderCode.c_str(), vertexShaderID))
	{
		std::stringstream ss;
		ss << "Shader::loadShaders(): Couldn't compile vertex shader!" << std::endl;
		ss << "vertex file: " << vertexFileDesc.filePath;
		throw_with_trace(ShaderInitException(ss.str()));
	}

	if (!compileShaderComponent(fragmentShaderCode.c_str(), fragmentShaderID))
	{
		std::stringstream ss;
		ss << "Shader::loadShaders(): Couldn't compile fragment shader!" << std::endl;
		ss << "fragment file: " << fragmentFileDesc.filePath;
		throw_with_trace(ShaderInitException(ss.str()));
	}

	if (useGeomtryShader)
	{
		if (!compileShaderComponent(geometryShaderCode.c_str(), geometryShaderID))
		{
			std::stringstream ss;
			ss << "Shader::loadShaders(): Couldn't compile geometry shader!" << std::endl;
			ss << "geometry file: " << geometryFileDesc.filePath;
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

FileSystem * ShaderProgramGL::getShaderFileSystem()
{
	static FileSystem shaderFileSystem;
	return &shaderFileSystem;
}

ShaderSourceFileGenerator* ShaderProgramGL::getSourceFileGenerator()
{
	static ShaderSourceFileGenerator generator(getShaderFileSystem());
	return &generator;
}


bool ShaderProgramGL::compileShaderComponent(const std::string& shaderContent, GLuint shaderResourceID)
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

void ShaderProgramGL::bind()
{
	GLCall(glUseProgram(programID));
	mIsBound = true;
}


std::string ShaderProgramGL::adjustLineNumbers(char* message, const ProgramDesc& desc)
{
	if (message == nullptr) return "";

	enum class ParseState
	{
		None = 0, LineNumber = 1
	};

	ParseState state = ParseState::None;

	int i = 0;

	std::stringstream result;
	std::stringstream messageLine;
	std::stringstream lineNumber;

	while (message[i] != '\0')
	{
		char c = message[i];
		++i;

		// Ignore carriage return
		if (c == '\r') continue;

		// New line?
		if (c == '\n')
		{
			//LOG(demo::Logger()) << "Finished line: " << messageLine.str();
			result << messageLine.str() << std::endl;
			messageLine.str("");
			continue;
		}

		if (state == ParseState::None)
			messageLine << c;

		if (state == ParseState::None && messageLine.str() == "ERROR: 0:")
		{
			//LOG(demo::Logger())<< "Detected ERROR: 0: statement";
			state = ParseState::LineNumber;
		}
		else if (state == ParseState::LineNumber && c != ':')
		{
			lineNumber << c;

		}
		else if (state == ParseState::LineNumber && c == ':')
		{
			//LOG(demo::Logger()) << "Read line number: " << lineNumber.str();

			int lineNumberInt = atoi(lineNumber.str().c_str());
			int column = 1;
			unsigned int resolvedPosition = ShaderSourceFileGenerator::calcResolvedPosition(desc, lineNumberInt - 1, column - 1);
			ReverseInfo reverseInfo = ShaderSourceFileGenerator::reversePosition(&desc.root, resolvedPosition);
			ShaderSourceFileGenerator::calcLineColumn(reverseInfo.fileDesc->source, reverseInfo.position, &lineNumberInt, &column);
			++lineNumberInt;
			++column;

			// Adjust the linenumber by offset if the error occurred in the root file
			if (reverseInfo.fileDesc == &desc.root)
			{
				lineNumberInt += desc.parseErrorLogOffset;
			}

			messageLine.str("");
			messageLine << "ERROR: " << reverseInfo.fileDesc->filePath << ", line " << lineNumberInt << ": ";

			lineNumber.str("");
			state = ParseState::None;
		}
	}

	return result.str();
}

GLuint ShaderProgramGL::compileShader(unsigned type, const ProgramDesc& desc)
{
	static Logger logger("[Shader]");

	GLuint id;
	GLCall(id = glCreateShader(type));

	const char* src = desc.root.resolvedSource.data();

	GLCall(glShaderSource(id, 1, &src, nullptr));
	GLCall(glCompileShader(id));

	// TODO error handling

	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		GLCall(glGetShaderInfoLog(id, length, &length, message));

		std::string modifiedMessage = adjustLineNumbers(message, desc);

		logger.log(Error) << modifiedMessage;

		std::string shaderType = type == GL_VERTEX_SHADER ? "vertex" : "fragment";

		GLCall(glDeleteShader(id));
		throw std::runtime_error("Failed to compile " + shaderType + " shader!");
		//return GL_FALSE;
	}

	return id;
}

GLuint ShaderProgramGL::createShader(const ProgramDesc& vertexShader, const ProgramDesc& fragmentShader)
{
	GLuint program, vs, fs;
	GLCall(program = glCreateProgram());
	GLCall(vs = compileShader(GL_VERTEX_SHADER, vertexShader));
	GLCall(fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader));

	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	GLCall(glValidateProgram(program));

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));

	return program;
}

bool ShaderProgramGL::extractIncludeStatement(const std::string& line, std::string* result)
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

bool ShaderProgramGL::isValid(unsigned textureUnit)
{
	int unitCount;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &unitCount);
	return textureUnit < unitCount;
}

std::string ShaderProgramGL::loadShaderComponent(const std::string& shaderFile, bool writeUnfoldedResult, std::vector<std::string> defines)
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

std::vector<std::string> ShaderProgramGL::preprocess(std::string& shaderContent, const std::vector<std::string>& defines)
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

void ShaderProgramGL::writeUnfoldedShaderContentToFile(const std::string& shaderSourceFile, const std::vector<std::string>& lines)
{
	nex::filesystem::writeToFile(shaderSourceFile + ".unfolded", lines, std::ostream::trunc);
}

ShaderGL::ShaderGL() : mProgram(nullptr)
{
}


ShaderGL::~ShaderGL()
{
	delete mProgram;
}


void ShaderGL::bind()
{
	mProgram->bind();
}

ShaderProgramGL* ShaderGL::getProgram()
{
	return mProgram;
}

void ShaderGL::setProgram(ShaderProgramGL* program)
{
	mProgram = program;
}

void ShaderGL::unbind()
{
	mProgram->unbind();
}

void ShaderGL::onModelMatrixUpdate(const glm::mat4& modelMatrix)
{
}

void ShaderGL::onMaterialUpdate(const Material* material)
{
}

void ShaderGL::setupRenderState()
{
}

void ShaderGL::reverseRenderState()
{
}