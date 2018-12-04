#include <string>
#include <vector>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/FileSystem.hpp>
#include <nex/exception/ShaderInitException.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <boost/filesystem.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>
#include <regex>
#include "nex/opengl/mesh/MeshGL.hpp"
#include "nex/shader_generator/SourceFileConsumer.hpp"
#include <nex/shader_generator/ShaderSourceFileGenerator.hpp>
#include <nex/texture/Texture.hpp>

using namespace nex;
using namespace ::util;
using namespace glm;

Logger staticLogClient("ShaderGL-static");


ShaderStageGL translate(nex::ShaderStageType stage)
{
	static ShaderStageGL const table[] =
	{
		ShaderStageGL::COMPUTE,
		ShaderStageGL::FRAGMENT,
		ShaderStageGL::GEOMETRY,
		ShaderStageGL::TESSELATION_CONTROL,
		ShaderStageGL::TESSELATION_EVALUATION,
		ShaderStageGL::VERTEX,
	};

	static const unsigned size = static_cast<unsigned>(ShaderStageType::SHADER_STAGE_LAST) + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "NeX error: ShaderStageGL descriptor list doesn't match number of supported shader stages");

	return table[static_cast<unsigned>(stage)];
}

ShaderStageType translate(ShaderStageGL stage)
{
	switch (stage)
	{
		case ShaderStageGL::VERTEX: return ShaderStageType::VERTEX;
		case ShaderStageGL::FRAGMENT: return ShaderStageType::FRAGMENT;
		case ShaderStageGL::GEOMETRY: return ShaderStageType::GEOMETRY;
		case ShaderStageGL::COMPUTE: return ShaderStageType::COMPUTE;
		case ShaderStageGL::TESSELATION_CONTROL: return ShaderStageType::TESSELATION_CONTROL;
		case ShaderStageGL::TESSELATION_EVALUATION: return ShaderStageType::TESSELATION_EVALUATION;
	default:;
	}

	throw_with_trace(std::runtime_error("Unknown ShaderStageGL detected on ShaderGL translation!"));

	// won't be reached
	return ShaderStageType::VERTEX;
}

std::ostream& operator<<(std::ostream& os, ShaderStageGL type)
{
	switch (type)
	{
		case ShaderStageGL::VERTEX: os << "vertex";  break;
		case ShaderStageGL::FRAGMENT: os << "fragment"; break;
		case ShaderStageGL::GEOMETRY: os << "geometry"; break;
		case ShaderStageGL::COMPUTE: os << "compute"; break;
		case ShaderStageGL::TESSELATION_CONTROL:os << "tesselation control"; break;
		case ShaderStageGL::TESSELATION_EVALUATION:os << "tesselation evaluation"; break;
	default:;
	}

	return os;
}

nex::ShaderProgram::ShaderProgram(): mIsBound(false)
{
}


void nex::ShaderProgram::setInt(const UniformLocation* locationID, int data)
{
	assert(mIsBound);
	if ((GLint)locationID < 0) return;
	GLCall(glUniform1i((GLint)locationID, data));
}

void nex::ShaderProgram::setFloat(const UniformLocation* locationID, float data)
{
	assert(mIsBound);
	if ((GLint)locationID < 0) return;
	GLCall(glUniform1f((GLint)locationID, data));
}

void nex::ShaderProgram::setVec2(const UniformLocation* locationID, const glm::vec2& data)
{
	assert(mIsBound);
	if ((GLint)locationID < 0) return;;
	GLCall(glUniform2f((GLint)locationID, data.x, data.y));
}

void nex::ShaderProgram::setVec3(const UniformLocation* locationID, const glm::vec3& data)
{
	ASSERT(mIsBound);
	if ((GLint)locationID < 0) return;

	//GLClearError();
	GLCall(glUniform3f((GLint)locationID, data.x, data.y, data.z));

	/*if (!GLLogCall())
	{
		
		nex::Logger("ShaderProgramGL")(nex::Error) << "Something went wrong!";
	}*/
}

void nex::ShaderProgram::setVec4(const UniformLocation* locationID, const glm::vec4& data)
{
	assert(mIsBound);
	if ((GLint)locationID < 0) return;

	GLCall(glUniform4f((GLint)locationID, data.x, data.y, data.z, data.w));
}

void nex::ShaderProgram::setMat3(const UniformLocation* locationID, const glm::mat3& data)
{
	assert(mIsBound);
	if ((GLint)locationID < 0) return;
	GLCall(glUniformMatrix3fv((GLint)locationID, 1, GL_FALSE, value_ptr(data)));
}

void nex::ShaderProgram::setMat4(const UniformLocation* locationID, const glm::mat4& data)
{
	assert(mIsBound);
	if ((GLint)locationID < 0) return;
	GLCall(glUniformMatrix4fv((GLint)locationID, 1, GL_FALSE, value_ptr(data)));
}

//void setTexture(const UniformLocation* locationID, const Texture* data, unsigned int bindingSlot);
void nex::ShaderProgram::setTexture(const UniformLocation* locationID, const nex::Texture* data, unsigned int bindingSlot)
{
	//assert(mIsBound);
	ASSERT(mIsBound);
	//ASSERT(isValid(textureSlot));
	if ((GLint)locationID < 0) return;

	TextureGL* gl = (TextureGL*)data->getImpl();

	GLCall(glBindTextureUnit(bindingSlot, *gl->getTexture()));
	GLCall(glUniform1i((GLint)locationID, bindingSlot));
}

void nex::ShaderProgram::setDebugName(const char* name)
{
	mDebugName = name;
}

void nex::ShaderProgram::updateBuffer(const UniformLocation* locationID, void* data, size_t bufferSize)
{
	// Not implemented yet!
	assert(false);
}


GLuint nex::ShaderProgramGL::getProgramID() const
{
	return programID;
}

UniformLocation* nex::ShaderProgram::getUniformLocation(const char* name)
{
	ShaderProgramGL* thiss = (ShaderProgramGL*)this;

	auto loc = glGetUniformLocation(thiss->programID, name);

	if (loc < 0)
	{
		static Logger logger("ShaderProgramGL::getUniformLocation");
		logger(Debug) << "Uniform '" << name << "' doesn't exist in shader '" << mDebugName << "'";
	}

	return (UniformLocation*)loc;
}

ShaderProgram* nex::ShaderProgram::create(const FilePath& vertexFile, const FilePath& fragmentFile, const FilePath& geometryShaderFile)
{
	GLuint programID = ShaderProgramGL::loadShaders(vertexFile, fragmentFile, geometryShaderFile);
	if (programID == GL_FALSE)
	{
		throw_with_trace(ShaderInitException("ShaderProgram::create: couldn't create shader!"));
	}

	return new ShaderProgramGL(programID);
}

void nex::ShaderProgram::release()
{
	ShaderProgramGL* thiss = (ShaderProgramGL*)this;
	if (thiss->programID != GL_FALSE)
	{
		GLCall(glDeleteProgram(thiss->programID));
		thiss->programID = GL_FALSE;
	}
}

void nex::ShaderProgram::unbind()
{
	GLCall(glUseProgram(0));
	mIsBound = false;
}

GLuint nex::ShaderProgramGL::loadShaders(const std::string& vertexFile, const std::string& fragmentFile,
	const std::string& geometryShaderFile)
{	
	bool useGeometryShader = geometryShaderFile.compare("") != 0;

	ShaderStageDesc vertexDesc;
	ShaderStageDesc fragmentDesc;
	ShaderStageDesc geometryDesc;
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


		shaderProgram = createShaderProgram(vertexDesc, fragmentDesc, &geometryDesc);


	} else
	{
		shaderProgram = createShaderProgram(vertexDesc, fragmentDesc, nullptr);
	}

	/*if (writeUnfoldedResult)
		writeUnfoldedShaderContentToFile(shaderFile, lines);*/

	return shaderProgram;
}

nex::ShaderProgramGL::ShaderProgramGL(GLuint program) : programID(program)
{
}

nex::ShaderProgramGL::~ShaderProgramGL()
{
	if (programID != GL_FALSE)
		GLCall(glDeleteProgram(programID));
	programID = GL_FALSE;
}

void nex::ShaderProgram::bind()
{
	GLCall(glUseProgram(((ShaderProgramGL*)this)->programID));
	mIsBound = true;
}


std::string nex::ShaderProgramGL::adjustLineNumbers(char* message, const ShaderStageDesc& desc)
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

			size_t lineNumberInt = atol(lineNumber.str().c_str());
			size_t column = 1;
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

GLuint nex::ShaderProgramGL::compileShaderStage(unsigned int type, const ShaderStageDesc& desc)
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

GLuint nex::ShaderProgramGL::createShaderProgram(const ShaderStageDesc& vertexShader, const ShaderStageDesc& fragmentShader, const ShaderStageDesc* geometryShader)
{
	GLuint program, vs, fs, gs;
	bool useGeometryShader = geometryShader != nullptr;
	int infoLogLength;
	GLint result = GL_FALSE;


	GLCall(program = glCreateProgram());
	GLCall(vs = compileShaderStage(GL_VERTEX_SHADER, vertexShader));
	GLCall(fs = compileShaderStage(GL_FRAGMENT_SHADER, fragmentShader));
	
	if (useGeometryShader)
	{
		GLCall(gs = compileShaderStage(GL_GEOMETRY_SHADER, *geometryShader));
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

void nex::ShaderProgramGL::writeUnfoldedShaderContentToFile(const std::string& shaderSourceFile, const std::vector<char>& sourceCode)
{
	FileSystem::writeToFile(shaderSourceFile + ".unfolded", sourceCode, std::ostream::trunc);
}