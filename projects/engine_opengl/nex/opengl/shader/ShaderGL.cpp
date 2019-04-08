#include <string>
#include <vector>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/FileSystem.hpp>
#include <nex/exception/ShaderInitException.hpp>
#include <boost/filesystem.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <regex>
#include <nex/shader_generator/SourceFileConsumer.hpp>
#include <nex/shader_generator/ShaderSourceFileGenerator.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/opengl.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/opengl/RenderBackendGL.hpp>

using namespace glm;

nex::Logger staticLogClient("ShaderGL-static");


nex::ShaderStageTypeGL nex::translate(nex::ShaderStageType stage)
{
	static ShaderStageTypeGL const table[] =
	{
		ShaderStageTypeGL::COMPUTE,
		ShaderStageTypeGL::FRAGMENT,
		ShaderStageTypeGL::GEOMETRY,
		ShaderStageTypeGL::TESSELATION_CONTROL,
		ShaderStageTypeGL::TESSELATION_EVALUATION,
		ShaderStageTypeGL::VERTEX,
	};

	static const unsigned size = static_cast<unsigned>(ShaderStageType::SHADER_STAGE_LAST) + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "NeX error: ShaderStageGL descriptor list doesn't match number of supported shader stages");

	return table[static_cast<unsigned>(stage)];
}

nex::ShaderStageType nex::translate(ShaderStageTypeGL stage)
{
	switch (stage)
	{
		case ShaderStageTypeGL::VERTEX: return ShaderStageType::VERTEX;
		case ShaderStageTypeGL::FRAGMENT: return ShaderStageType::FRAGMENT;
		case ShaderStageTypeGL::GEOMETRY: return ShaderStageType::GEOMETRY;
		case ShaderStageTypeGL::COMPUTE: return ShaderStageType::COMPUTE;
		case ShaderStageTypeGL::TESSELATION_CONTROL: return ShaderStageType::TESSELATION_CONTROL;
		case ShaderStageTypeGL::TESSELATION_EVALUATION: return ShaderStageType::TESSELATION_EVALUATION;
	default:;
	}

	throw_with_trace(std::runtime_error("Unknown ShaderStageGL detected on ShaderGL translation!"));

	// won't be reached
	return ShaderStageType::VERTEX;
}

std::ostream& nex::operator<<(std::ostream& os, ShaderStageTypeGL type)
{
	switch (type)
	{
		case ShaderStageTypeGL::VERTEX: os << "vertex";  break;
		case ShaderStageTypeGL::FRAGMENT: os << "fragment"; break;
		case ShaderStageTypeGL::GEOMETRY: os << "geometry"; break;
		case ShaderStageTypeGL::COMPUTE: os << "compute"; break;
		case ShaderStageTypeGL::TESSELATION_CONTROL:os << "tesselation control"; break;
		case ShaderStageTypeGL::TESSELATION_EVALUATION:os << "tesselation evaluation"; break;
	default:;
	}

	return os;
}


void nex::ComputeShader::dispatch(unsigned workGroupsX, unsigned workGroupsY, unsigned workGroupsZ)
{
	GLCall(glDispatchCompute(workGroupsX, workGroupsY, workGroupsZ));
}


void nex::ShaderProgram::setImageLayerOfTexture(UniformLocation locationID, const nex::Texture* data, unsigned int bindingSlot,
	TextureAccess accessType, InternFormat format, unsigned level, bool textureIsArray, unsigned layer)
{
	mImpl->setImageLayerOfTexture(locationID, data, bindingSlot, accessType, format, level, textureIsArray, layer);
}


void nex::ShaderProgram::setInt(UniformLocation locationID, int data)
{
	mImpl->setInt(locationID, data);
}

void nex::ShaderProgram::setFloat(UniformLocation locationID, float data)
{
	mImpl->setFloat(locationID, data);
}

void nex::ShaderProgram::setUInt(UniformLocation locationID, unsigned data)
{
	mImpl->setUInt(locationID, data);
}

void nex::ShaderProgram::setUVec2(UniformLocation locationID, const glm::uvec2& data)
{
	mImpl->setUVec2(locationID, data);
}

void nex::ShaderProgram::setUVec3(UniformLocation locationID, const glm::uvec3& data)
{
	mImpl->setUVec3(locationID, data);
}

void nex::ShaderProgram::setUVec4(UniformLocation locationID, const glm::uvec4& data)
{
	mImpl->setUVec4(locationID, data);
}

void nex::ShaderProgram::setVec2(UniformLocation locationID, const glm::vec2& data)
{
	mImpl->setVec2(locationID, data);
}

void nex::ShaderProgram::setVec3(UniformLocation locationID, const glm::vec3& data)
{
	mImpl->setVec3(locationID, data);
}

void nex::ShaderProgram::setVec4( UniformLocation locationID, const glm::vec4& data)
{
	mImpl->setVec4(locationID, data);
}

void nex::ShaderProgram::setMat3( UniformLocation locationID, const glm::mat3& data)
{
	mImpl->setMat3(locationID, data);
}

void nex::ShaderProgram::setMat4(UniformLocation locationID, const glm::mat4& data)
{
	mImpl->setMat4(locationID, data);
}

//void setTexture(const UniformLocation* locationID, const Texture* data, unsigned int bindingSlot);
void nex::ShaderProgram::setTexture(UniformLocation locationID, const nex::Texture* data, unsigned int bindingSlot)
{
	mImpl->setTexture(locationID, data, bindingSlot);
}

void nex::ShaderProgram::setDebugName(const char* name)
{
	mImpl->setDebugName(name);
}

nex::ShaderStage* nex::ShaderStage::compileShaderStage(const nex::ResolvedShaderStageDesc& desc)
{
	static Logger logger("nex::ShaderStage::compileShaderStage");
	LOG(logger, LogLevel::Info) << "compiling " << desc.root.filePath;
	GLuint id;
	GLCall(id = glCreateShader(translate(desc.type)));

	const char* src = desc.root.resolvedSource.data();
	const GLint size = static_cast<GLint>(desc.root.resolvedSource.size());

	GLCall(glShaderSource(id, 1, &src, &size));
	GLCall(glCompileShader(id));

	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		GLCall(glGetShaderInfoLog(id, length, &length, message));

		std::string modifiedMessage = ShaderProgram::Impl::adjustLineNumbers(message, desc);

		LOG(staticLogClient, Error) << modifiedMessage;

		GLCall(glDeleteShader(id));

		std::stringstream ss;
		ss << "Failed to compile " << desc.type << " shader!";

		throw_with_trace(std::runtime_error(ss.str()));
		//return GL_FALSE;
	}

	return new ShaderStageGL(id, desc.type);
}


nex::ShaderStageGL::ShaderStageGL(GLuint shaderStage, nex::ShaderStageType type) : ShaderStage(type), mShaderStage(shaderStage)
{
}

nex::ShaderStageGL::~ShaderStageGL()
{
	if (mShaderStage != GL_FALSE)
	{
		GLCall(glDeleteShader(mShaderStage));
		mShaderStage = GL_FALSE;
	}
}

GLuint nex::ShaderStageGL::getID() const
{
	return mShaderStage;
}

nex::ShaderProgram::ShaderProgram(std::unique_ptr<Impl> impl) : mImpl(std::move(impl))
{
}

nex::ShaderProgram::Impl::Impl(GLuint program) : mProgramID(program), mCache(program)
{
}

nex::ShaderProgram::Impl::~Impl()
{
	if (mProgramID != GL_FALSE)
	{
		GLCall(glDeleteProgram(mProgramID));
		mProgramID = GL_FALSE;
	}
}

void nex::ShaderProgram::Impl::bind()
{
	mCache.UseProgram();
}

GLuint nex::ShaderProgram::Impl::getProgramID() const
{
	return mProgramID;
}

nex::UniformLocation nex::ShaderProgram::Impl::getShaderStorageBufferLocation(const char* name) const
{
	GLCall(const auto loc = glGetProgramResourceIndex(mProgramID, GL_SHADER_STORAGE_BLOCK, name));

	if (loc == GL_INVALID_INDEX)
	{
		static Logger logger("ShaderProgramGL::getUniformLocation");
		logger(Debug) << "Uniform '" << name << "' doesn't exist in shader '" << mDebugName << "'";
	}

	return loc;
}

nex::UniformLocation nex::ShaderProgram::Impl::getUniformBufferLocation(const char* name) const
{
	GLCall(const auto loc = glGetUniformBlockIndex(mProgramID, name));

	if (loc == GL_INVALID_INDEX)
	{
		static Logger logger("ShaderProgramGL::getUniformBufferLocation");
		logger(Debug) << "Uniform '" << name << "' doesn't exist in shader '" << mDebugName << "'";
	}

	return loc;
}

nex::UniformLocation nex::ShaderProgram::Impl::getUniformLocation(const char* name) const
{
	GLCall(const auto loc = glGetUniformLocation(mProgramID, name));

	if (loc == GL_INVALID_INDEX)
	{
		static Logger logger("ShaderProgramGL::getUniformLocation");
		logger(Debug) << "Uniform '" << name << "' doesn't exist in shader '" << mDebugName << "'";
	}

	return loc;
}

void nex::ShaderProgram::Impl::setDebugName(const char* name)
{
	mDebugName = name;
}

void nex::ShaderProgram::Impl::setImageLayerOfTexture(UniformLocation locationID, const nex::Texture* data,
	unsigned bindingSlot, TextureAccess accessType, InternFormat format, unsigned level, bool textureIsArray,
	unsigned layer)
{
	GLint glID = locationID;
	if (glID < 0) return;

	auto* gl = data->getImpl();

	GlobalCacheGL::get()->BindImageTexture(bindingSlot,
		*gl->getTexture(),
		level,
		translate(textureIsArray),
		layer,
		(GLenum)translate(accessType),
		(GLenum)translate(format));

	mCache.Uniform1i(glID, bindingSlot);
}

void nex::ShaderProgram::Impl::setFloat(UniformLocation locationID, float data)
{
	GLint glID = locationID;
	if (glID < 0) return;
	mCache.Uniform1f(glID, data);
}

void nex::ShaderProgram::Impl::setInt(UniformLocation locationID, int data)
{
	GLint glID = locationID;
	if (glID < 0) return;
	mCache.Uniform1i(glID, data);
}

void nex::ShaderProgram::Impl::setTexture(UniformLocation locationID, const Texture* data, unsigned bindingSlot)
{
	GLint glID = locationID;
	if (glID < 0) return;

	auto* gl = data->getImpl();

	GlobalCacheGL::get()->BindTextureUnit(bindingSlot, *gl->getTexture());
	mCache.Uniform1i(glID, bindingSlot);
}

void nex::ShaderProgram::Impl::setUInt(UniformLocation locationID, unsigned data)
{
	GLint glID = locationID;
	if (glID < 0) return;
	mCache.Uniform1ui(glID, data);
}

void nex::ShaderProgram::Impl::setMat3(UniformLocation locationID, const glm::mat3& data)
{
	GLint glID = locationID;
	if (glID < 0) return;
	GLCall(glUniformMatrix3fv(glID, 1, GL_FALSE, value_ptr(data)));
}

void nex::ShaderProgram::Impl::setMat4(UniformLocation locationID, const glm::mat4 & data)
{
	GLint glID = locationID;
	if (glID < 0) return;
	GLCall(glUniformMatrix4fv(glID, 1, GL_FALSE, value_ptr(data)));
}

void nex::ShaderProgram::Impl::setVec2(UniformLocation locationID, const glm::vec2& data)
{
	GLint glID = locationID;
	if (glID < 0) return;

	GLCall(glUniform2f(glID, data.x, data.y));
}

void nex::ShaderProgram::Impl::setVec3(UniformLocation locationID, const glm::vec3& data)
{
	GLint glID = locationID;
	if (glID < 0) return;

	GLCall(glUniform3f(glID, data.x, data.y, data.z));
}

void nex::ShaderProgram::Impl::setVec4(UniformLocation locationID, const glm::vec4& data)
{
	GLint glID = locationID;
	if (glID < 0) return;

	GLCall(glUniform4f(glID, data.x, data.y, data.z, data.w));
}

void nex::ShaderProgram::Impl::setUVec2(UniformLocation locationID, const glm::uvec2& data)
{
	GLint glID = locationID;
	if (glID < 0) return;

	GLCall(glUniform2ui(glID, data.x, data.y));
}

void nex::ShaderProgram::Impl::setUVec3(UniformLocation locationID, const glm::uvec3& data)
{
	GLint glID = locationID;
	if (glID < 0) return;

	GLCall(glUniform3ui(glID, data.x, data.y, data.z));
}

void nex::ShaderProgram::Impl::setUVec4(UniformLocation locationID, const glm::uvec4& data)
{
	GLint glID = locationID;
	if (glID < 0) return;

	GLCall(glUniform4ui(glID, data.x, data.y, data.z, data.w));
}

void nex::ShaderProgram::Impl::unbind()
{
	GlobalCacheGL::get()->UseProgram(0);
}

nex::UniformLocation nex::ShaderProgram::getUniformBufferLocation(const char* name) const
{
	return mImpl->getUniformBufferLocation(name);
}

nex::UniformLocation nex::ShaderProgram::getShaderStorageBufferLocation(const char* name) const
{
	return mImpl->getShaderStorageBufferLocation(name);
}

nex::UniformLocation nex::ShaderProgram::getUniformLocation(const char* name) const
{
	return mImpl->getUniformLocation(name);
}

std::unique_ptr<nex::ShaderProgram> nex::ShaderProgram::create(const FilePath& vertexFile, const FilePath& fragmentFile, const FilePath& geometryShaderFile, 
	const std::vector<std::string>& defines)
{

	bool useGeometryShader = std::filesystem::exists(geometryShaderFile);

	std::vector<UnresolvedShaderStageDesc> unresolved;

	if (useGeometryShader)
		unresolved.resize(3);
	else
		unresolved.resize(2);


	unresolved[0].filePath = vertexFile;
	unresolved[0].type = ShaderStageType::VERTEX;
	unresolved[0].defines = defines;
	unresolved[1].filePath = fragmentFile;
	unresolved[1].type = ShaderStageType::FRAGMENT;
	unresolved[1].defines = defines;

	if (useGeometryShader)
	{
		unresolved[2].filePath = geometryShaderFile;
		unresolved[2].type = ShaderStageType::GEOMETRY;
		unresolved[3].defines = defines;
	}



	GLuint programID = ShaderProgram::Impl::loadShaders(unresolved);
	if (programID == GL_FALSE)
	{
		throw_with_trace(ShaderInitException("ShaderProgram::create: couldn't create shader!"));
	}

	return std::make_unique<ShaderProgram>(std::make_unique<ShaderProgram::Impl>(programID));
}

std::unique_ptr<nex::ShaderProgram> nex::ShaderProgram::create(const std::vector<Guard<ShaderStage>>& stages)
{
	GLuint programID = Impl::createShaderProgram(stages);
	return std::make_unique<ShaderProgram>(std::make_unique<ShaderProgram::Impl>(programID));
}

void nex::ShaderProgram::unbind()
{
	mImpl->unbind();
}

GLuint nex::ShaderProgram::Impl::loadShaders(const std::vector<UnresolvedShaderStageDesc>& stageDescs)
{	
	ShaderSourceFileGenerator* generator = ShaderSourceFileGenerator::get(); 
	ProgramSources programSources = generator->generate(stageDescs);

	std::vector<Guard<ShaderStage>> shaderStages;
	shaderStages.resize(programSources.descs.size());
	for (auto i = 0;  i < shaderStages.size(); ++i)
	{
		shaderStages[i] = ShaderStage::compileShaderStage(programSources.descs[i]);
	}

	return createShaderProgram(shaderStages);
}

void nex::ShaderProgram::bind()
{
	mImpl->bind();
}


std::string nex::ShaderProgram::Impl::adjustLineNumbers(char* message, const ResolvedShaderStageDesc& desc)
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

			// calc line and column number
			ShaderSourceFileGenerator::calcLineColumn(reverseInfo.fileDesc->source, reverseInfo.position, &lineNumberInt, &column);
			++lineNumberInt;
			++column;


			// Adjust the linenumber by offset if the error occurred in the root file
			if (reverseInfo.fileDesc == &desc.root)
			{
				lineNumberInt += desc.parseErrorLogOffset;
			}

			messageLine.str("");

			if (reverseInfo.errorInUserDefineMacro)
			{
				messageLine << "ERROR: " << reverseInfo.fileDesc->filePath << ", User provided define macro " << ": ";
			} else
			{
				messageLine << "ERROR: " << reverseInfo.fileDesc->filePath << ", line " << lineNumberInt << ": ";
			}

			lineNumber.str("");
			state = ParseState::None;
		}
	}

	return result.str();
}



GLuint nex::ShaderProgram::Impl::compileShaderStage(const ResolvedShaderStageDesc& desc, ShaderStageType type)
{
	GLuint id;
	GLCall(id = glCreateShader(translate(type)));

	const char* src = desc.root.resolvedSource.data();

	GLCall(glShaderSource(id, 1, &src, nullptr));
	GLCall(glCompileShader(id));

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

		GLCall(glDeleteShader(id));

		std::stringstream ss;
		ss << "Failed to compile " << type << " shader!";

		throw_with_trace(std::runtime_error(ss.str()));
		//return GL_FALSE;
	}

	return id;
}

GLuint nex::ShaderProgram::Impl::createShaderProgram(const std::vector<Guard<ShaderStage>>& stages)
{
	GLuint program;
	int infoLogLength;
	GLint result = GL_FALSE;

	GLCall(program = glCreateProgram());
	for (auto& stage : stages)
	{
		ShaderStageGL* stageGL = (ShaderStageGL*)stage.get();
		GLCall(glAttachShader(program, stageGL->getID()));
	}

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

		// release progrom
		GLCall(glDeleteProgram(program));

		throw_with_trace(ShaderInitException("Error: Shader::loadShaders(): Couldn't create shader program!"));
	}

	return program;
}

void nex::ShaderProgram::Impl::writeUnfoldedShaderContentToFile(const std::string& shaderSourceFile, const std::vector<char>& sourceCode)
{
	FileSystem::writeToFile(shaderSourceFile + ".unfolded", sourceCode, std::ostream::trunc);
}