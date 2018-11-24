#include <string>
#include <vector>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/FileSystem.hpp>
#include <nex/util/Globals.hpp>
#include <nex/exception/ShaderInitException.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <boost/filesystem.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>
#include <regex>
#include "nex/opengl/mesh/MeshGL.hpp"
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

std::ostream& operator<<(std::ostream& os, ShaderTypeGL type)
{
	switch (type)
	{
	case ShaderTypeGL::VERTEX: os << "vertex";  break;
	case ShaderTypeGL::FRAGMENT: os << "fragment"; break;
	case ShaderTypeGL::GEOMETRY: os << "geometry"; break;
	default:;
	}

	return os;
}

ShaderProgramGL::ShaderProgramGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile,
	const std::string& geometryShaderFile, const std::string& instancedVertexShaderFile) : mIsBound(false), mDebugName("ShaderProgramGL")
{
	programID = loadShaders(vertexShaderFile, fragmentShaderFile, geometryShaderFile);
	if (programID == GL_FALSE)
	{
		throw_with_trace(ShaderInitException("ShaderProgramGL::ShaderProgramGL: couldn't load shader"));
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

GLuint ShaderProgramGL::getProgramID() const
{
	return programID;
}

unsigned ShaderProgramGL::getUniformLocation(const char* name)
{
	auto loc = glGetUniformLocation(programID, name);

	if (loc < 0)
	{
		static Logger logger("ShaderProgramGL::getUniformLocation");
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
	bool useGeometryShader = geometryShaderFile.compare("") != 0;

	ProgramDesc vertexDesc;
	ProgramDesc fragmentDesc;
	ProgramDesc geometryDesc;
	GLuint shaderProgram;
	ShaderSourceFileGenerator* generator = getSourceFileGenerator();


	// Read the Vertex Shader code from file
	try
	{
		vertexDesc.root = generator->generate(vertexFile);
		writeUnfoldedShaderContentToFile(vertexDesc.root.filePath, vertexDesc.root.resolvedSource);
	} catch(const ParseException& e)
	{
		LOG(staticLogClient, Error) << e.what();
		throw_with_trace(ShaderInitException("Couldn't initialize vertex shader: " + vertexFile));
	}

	// Read the Fragment Shader code from file
	try
	{
		fragmentDesc.root = generator->generate(fragmentFile);
		writeUnfoldedShaderContentToFile(fragmentDesc.root.filePath, fragmentDesc.root.resolvedSource);
	}
	catch (const ShaderInitException e)
	{
		LOG(staticLogClient, Error) << e.what();
		throw_with_trace(ShaderInitException("Couldn't initialize fragment shader: " + fragmentFile));
	}


	if (useGeometryShader)
	{
		// Read the geometry Shader code from file
		try
		{
			geometryDesc.root = generator->generate(geometryShaderFile);
			writeUnfoldedShaderContentToFile(geometryDesc.root.filePath, geometryDesc.root.resolvedSource);
		}
		catch (const ShaderInitException e)
		{
			LOG(staticLogClient, Error) << e.what();
			throw_with_trace(ShaderInitException("Couldn't initialize geometry shader: " + geometryShaderFile));
		}


		shaderProgram = createShader(vertexDesc, fragmentDesc, &geometryDesc);


	} else
	{
		shaderProgram = createShader(vertexDesc, fragmentDesc, nullptr);
	}

	/*if (writeUnfoldedResult)
		writeUnfoldedShaderContentToFile(shaderFile, lines);*/

	return shaderProgram;
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

GLuint ShaderProgramGL::compileShader(unsigned int type, const ProgramDesc& desc)
{
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

		LOG(staticLogClient, Error) << modifiedMessage;

		//GLCall(glDeleteShader(id));

		std::stringstream ss;
		ss << "Failed to compile " << type << " shader!";

		throw_with_trace(std::runtime_error(ss.str()));
		//return GL_FALSE;
	}

	return id;
}

GLuint ShaderProgramGL::createShader(const ProgramDesc& vertexShader, const ProgramDesc& fragmentShader, const ProgramDesc* geometryShader)
{
	GLuint program, vs, fs, gs;
	bool useGeometryShader = geometryShader != nullptr;
	int infoLogLength;
	GLint result = GL_FALSE;


	GLCall(program = glCreateProgram());
	GLCall(vs = compileShader(GL_VERTEX_SHADER, vertexShader));
	GLCall(fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader));
	
	if (useGeometryShader)
	{
		GLCall(gs = compileShader(GL_GEOMETRY_SHADER, *geometryShader));
		GLCall(glAttachShader(program, gs));
	}

	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));

	GLCall(glLinkProgram(program));
	GLCall(glValidateProgram(program));

	// Check the program
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (result == GL_FALSE)
	{
		if (infoLogLength > 0) {
			std::vector<char> ProgramErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(program, infoLogLength, nullptr, &ProgramErrorMessage[0]);
			LOG(staticLogClient, Error) << &ProgramErrorMessage[0];
		}
		throw_with_trace(ShaderInitException("Error: Shader::loadShaders(): Couldn't create shader program!"));
	}

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));

	if (useGeometryShader)
	{
		GLCall(glDeleteShader(gs));
	}

	return program;
}

void ShaderProgramGL::writeUnfoldedShaderContentToFile(const std::string& shaderSourceFile, const std::vector<char>& sourceCode)
{
	FileSystem::writeToFile(shaderSourceFile + ".unfolded", sourceCode, std::ostream::trunc);
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