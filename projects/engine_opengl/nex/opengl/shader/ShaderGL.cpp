#include <string>
#include <vector>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/FileSystem.hpp>
#include <nex/util/Globals.hpp>
#include <nex/exception/ShaderInitException.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <fstream>
#include <boost/filesystem.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace nex;
using namespace ::util;
using namespace glm;

LoggingClient staticLogClient(getLogServer());


ShaderAttributeGL::ShaderAttributeGL()
{
	data = nullptr;
	type = ShaderAttributeType::MAT4;
	uniformName = "";
	m_isActive = false;
}

ShaderAttributeGL::ShaderAttributeGL(const ShaderAttributeGL& o) : ShaderAttribute(o), 
	uniformName(o.uniformName)
{
}

ShaderAttributeGL::ShaderAttributeGL(ShaderAttributeGL&& o) : ShaderAttribute(o), 
uniformName(o.uniformName)
{
}

ShaderAttributeGL& ShaderAttributeGL::operator=(const ShaderAttributeGL& o)
{
	if (this == &o) return *this;
	ShaderAttribute::operator=(o);
	uniformName = o.uniformName;
	return *this;
}

ShaderAttributeGL&& ShaderAttributeGL::operator=(ShaderAttributeGL&& o)
{
	if (this == &o) return std::move(*this);
	ShaderAttribute::operator=(std::move(o));
	uniformName = move(o.uniformName);
	return std::move(*this);
}

ShaderAttributeGL::ShaderAttributeGL(ShaderAttributeType type, const void* data, std::string uniformName, bool active)
{
	this->type = type;
	this->data = data;
	this->uniformName = uniformName;
	m_isActive = active;
}

ShaderAttributeGL::~ShaderAttributeGL(){}

const std::string& ShaderAttributeGL::getName() const
{
	return uniformName;
}

void ShaderAttributeGL::setData(const void* data)
{
	this->data = data;
}

void ShaderAttributeGL::setName(std::string name)
{
	uniformName = name;
}

void ShaderAttributeGL::setType(ShaderAttributeType type)
{
	this->type = type;
}

ShaderAttributeCollection::ShaderAttributeCollection(){}

ShaderAttributeCollection::ShaderAttributeCollection(const ShaderAttributeCollection& o) :
	vec(o.vec), lookup(o.lookup)
{}

ShaderAttributeCollection::ShaderAttributeCollection(ShaderAttributeCollection&& o) : 
	vec(o.vec), lookup(o.lookup)
{
	o.vec.clear();
	o.lookup.clear();
}

ShaderAttributeCollection& ShaderAttributeCollection::operator=(const ShaderAttributeCollection& o)
{
	if (this == &o)
		return *this;
	vec = o.vec;
	lookup = o.lookup;
	return *this;
}

ShaderAttributeCollection&& ShaderAttributeCollection::operator=(ShaderAttributeCollection&& o)
{
	if (this == &o)
		return std::move(*this);
	vec = move(o.vec);
	lookup = move(o.lookup);
	return std::move(*this);
}

ShaderAttributeCollection::~ShaderAttributeCollection(){}

ShaderAttributeCollection::ShaderAttributeKey ShaderAttributeCollection::create(ShaderAttributeType type, const void* data, std::string uniformName, bool active)
{
	vec.push_back({type, data, std::move(uniformName), active});
	auto result = &vec.back();
	lookup.insert({ result->getName(), (int)vec.size() - 1 });
	int val = (int)vec.size() - 1;
	return val;
}

ShaderAttributeGL* ShaderAttributeCollection::get(const std::string& uniformName)
{
	auto it = lookup.find(uniformName);
	if (it == lookup.end()) return nullptr;
	return &vec[it->second];
}

ShaderAttributeGL* ShaderAttributeCollection::get(ShaderAttributeKey key)
{
	return &vec[key];
}

const ShaderAttributeGL* ShaderAttributeCollection::getList() const
{
	return vec.data();
}

void ShaderAttributeCollection::setData(const std::string& uniformName, const void* data, const void* defaultValue, bool activate)
{
	auto attr = get(uniformName);

	//assert(attr != nullptr);
	if (attr == nullptr) {
		std::stringstream ss;
		ss << "Couldn't match uniform name >> " << uniformName << " << with a registered atrribute name.";
		throw_with_trace(std::runtime_error(ss.str()));
	}

	if (data == nullptr) {
		attr->setData(defaultValue);
	} else {
		attr->setData(data);
	}
	attr->activate(activate);
}

int ShaderAttributeCollection::size() const
{
	return (int)vec.size();
}

ShaderConfigGL::ShaderConfigGL(){}

ShaderConfigGL::~ShaderConfigGL(){}

void ShaderConfigGL::afterDrawing(const MeshGL& mesh){}

void ShaderConfigGL::beforeDrawing(const MeshGL& mesh){}

const ShaderAttribute* ShaderConfigGL::getAttributeList() const
{
	return attributes.getList();
}

int ShaderConfigGL::getNumberOfAttributes() const
{
	return attributes.size();
}

ShaderGL::ShaderGL(std::unique_ptr<ShaderConfigGL> config, const std::string& vertexShaderFile, const std::string& fragmentShaderFile,
	const std::string& geometryShaderFile, const std::string& instancedVertexShaderFile)
	: config(std::move(config)), instancedProgramID(0), logClient(getLogServer()), textureCounter(0)
{
	programID = loadShaders(vertexShaderFile, fragmentShaderFile, geometryShaderFile);
	if (programID == GL_FALSE)
	{
		throw_with_trace(ShaderInitException("ShaderGL::ShaderGL: couldn't load shader"));
	}

	if (instancedVertexShaderFile != "")
	{
		instancedProgramID = loadShaders(instancedVertexShaderFile, fragmentShaderFile, geometryShaderFile);
		if (instancedProgramID == GL_FALSE)
		{
			throw_with_trace(ShaderInitException("ShaderGL::ShaderGL: couldn't load instanced shader"));
		}
	}
}

ShaderGL::ShaderGL(const std::string & vertexShaderFile, 
	const std::string & fragmentShaderFile, 
	const std::string & geometryShaderFile, 
	const std::string & instancedVertexShaderFile) 
	: 
	ShaderGL(nullptr, 
		vertexShaderFile, 
		fragmentShaderFile, 
		geometryShaderFile, 
		instancedVertexShaderFile)
{
}

ShaderGL::ShaderGL(ShaderGL&& other) : config(move(other.config)),
    programID(other.programID), instancedProgramID(other.instancedProgramID),
	logClient(std::move(other.logClient)), textureCounter(other.textureCounter)
{
	other.programID = GL_FALSE;
	other.instancedProgramID = GL_FALSE;
	other.textureCounter = GL_FALSE;
	other.config = nullptr;
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

	LOG(staticLogClient, Debug) << "Test log!";

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

GLuint ShaderGL::getProgramID() const
{
	return programID;
}

void ShaderGL::release()
{
	glDeleteProgram(programID);
	glDeleteProgram(instancedProgramID);
}

void ShaderGL::use()
{
	glUseProgram(this->programID);
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

	// Read the Vertex Shader code from the file
	if (!::filesystem::loadFileIntoString(vertexFilePath, &vertexShaderCode))
	{
		std::stringstream ss;
		ss << "Shader::loadShaders(): Couldn't initialize vertex shader!" << std::endl;
		ss << "vertex file: " << vertexFilePath;

		throw_with_trace(ShaderInitException(ss.str()));
	}

	if (!::filesystem::loadFileIntoString(fragmentFilePath, &fragmentShaderCode))
	{
		LOG(staticLogClient, Error) << "Couldn't initialize fragment shader!";
		std::stringstream ss;
		ss << "Shader::loadShaders(): Couldn't initialize fragment shader!" << std::endl;
		ss << "fragment file: " << fragmentFilePath;
		throw_with_trace(ShaderInitException(ss.str()));
	}

	if (useGeomtryShader)
	{
		if (!::filesystem::loadFileIntoString(geometryFilePath, &geometryShaderCode))
		{
			LOG(staticLogClient, Error) << "Couldn't initialize geometry shader!";
			std::stringstream ss;
			ss << "Shader::loadShaders(): Couldn't initialize geometry shader!" << std::endl;
			ss << "geometry shader file: " << geometryFilePath;
			throw_with_trace(ShaderInitException(ss.str()));
		}
	}

	if (!compileShader(vertexShaderCode.c_str(), vertexShaderID))
	{
		std::stringstream ss;
		ss << "Shader::loadShaders(): Couldn't compile vertex shader!" << std::endl;
		ss << "vertex file: " << vertexFilePath;
		throw_with_trace(ShaderInitException(ss.str()));
	}

	if (!compileShader(fragmentShaderCode.c_str(), fragmentShaderID))
	{
		std::stringstream ss;
		ss << "Shader::loadShaders(): Couldn't compile fragment shader!" << std::endl;
		ss << "fragment file: " << fragmentFilePath;
		throw_with_trace(ShaderInitException(ss.str()));
	}

	if (useGeomtryShader)
	{
		if (!compileShader(geometryShaderCode.c_str(), geometryShaderID))
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


bool ShaderGL::compileShader(const std::string& shaderContent, GLuint shaderResourceID)
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

	//glCompileShader(shaderResourceID);

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

void ShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	textureCounter = 0;

	beforeDrawing(mesh);

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	afterDrawing(mesh);
}

void ShaderGL::drawInstanced(Mesh const& meshOriginal, unsigned amount)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	textureCounter = 0;
	
	beforeDrawing(mesh);

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElementsInstanced(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr, amount);
	glBindVertexArray(0);

	afterDrawing(mesh);
}

ShaderConfig* ShaderGL::getConfig() const
{
	return config.get();
};

void ShaderGL::afterDrawing(const MeshGL& mesh)
{
	if (config != nullptr)
		config->afterDrawing(mesh);
}

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
}

void ShaderGL::setAttribute(GLuint program, const ShaderAttributeGL& attribute)
{
	if (!attribute.isActive()) return;
	auto name = attribute.getName();

	auto loc = glGetUniformLocation(program, name.c_str());

	if (loc == -1)
	{
		//throw runtime_error("ShaderGL::setAttribute(): " + name + " doesn't correspond to "
		//	+ " an active uniform variable!");
	}

	const void* data = attribute.getData();
	assert(data != nullptr);

	using t = ShaderAttributeType;

	switch(attribute.getType())
	{
	case t::CUBE_MAP: {
		assert(textureCounter < 32); // OpenGL allows up to 32 textures
		glActiveTexture(textureCounter + GL_TEXTURE0);
		// TODO CubeDepthMaps should be considered as well!

		const CubeMapGL* texture = reinterpret_cast<const CubeMapGL*>(data);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture->getCubeMap());
		glUniform1i(loc, textureCounter);
		// the next texture to bind gets the next slot
		++textureCounter;
		break;
	}
	case t::FLOAT: {
		glUniform1f(loc, *reinterpret_cast<const float*>(data));
		break;
	}
	case t::INT: {
		glUniform1i(loc, *reinterpret_cast<const int*>(data));
		break;
	}
	case t::MAT3: {
		glUniformMatrix3fv(loc, 1, GL_FALSE, value_ptr(*reinterpret_cast<const mat3*>(data)));
		break;
	}
	case t::MAT4: {
		glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(*reinterpret_cast<const mat4*>(data)));
		break;
	}
	case t::TEXTURE2D: {
		assert(textureCounter < 32); // OpenGL allows up to 32 textures
		glActiveTexture(textureCounter + GL_TEXTURE0);
		const TextureGL* texture = reinterpret_cast<const TextureGL*>(data);
		glBindTexture(GL_TEXTURE_2D, texture->getTexture());
		glUniform1i(loc, textureCounter);
		// the next texture to bind gets the next slot
		++textureCounter;
		break;
	}
	case t::VEC2: {
		const vec2* vec = reinterpret_cast<const vec2*>(data);
		glUniform2f(loc, vec->x, vec->y);
		break;
	}
	case t::VEC3: {
		const vec3* vec = reinterpret_cast<const vec3*>(data);
		glUniform3f(loc, vec->x, vec->y, vec->z);
		break;
	}
	case t::VEC4: {
		const vec4* vec = reinterpret_cast<const vec4*>(data);
		glUniform4f(loc, vec->x, vec->y, vec->z, vec->w);
		break;
	}
	default:
		// TODO
		throw_with_trace(std::runtime_error("ShaderGL::setAttribute(): Unknown ShaderAttributeType: "));
	}

	RendererOpenGL::checkGLErrorSilently();
}