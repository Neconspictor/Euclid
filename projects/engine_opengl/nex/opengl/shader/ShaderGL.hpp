#pragma once
#include <string>
#include <glad/glad.h>
#include <nex/exception/ShaderInitException.hpp>
#include <nex/common/Log.hpp>
#include <nex/shader_generator/ShaderSourceFileGenerator.hpp>

class Material;
class CubeMapGL;
class TextureGL;
class MeshGL;
class Vob;

struct TransformData
{
	glm::mat4 const* projection = nullptr;
	glm::mat4 const* view = nullptr;
	glm::mat4 const* model = nullptr;
};

/**
* Enumerates all shaders that can be used for shading models.
*/
enum class ShaderType
{
	Unknown = 0,
	BlinnPhongTex,
	Pbr,
	Pbr_Deferred_Geometry,
	Pbr_Deferred_Lighting,
	Pbr_Convolution,
	Pbr_Prefilter,
	Pbr_BrdfPrecompute,
	CubeDepthMap,
	DepthMap,
	GaussianBlurHorizontal,
	GaussianBlurVertical,
	Normals,
	Shadow,
	ShadowPoint,
	SimpleColor,
	SimpleExtrude,
	Screen,
	SkyBox,
	SkyBoxEquirectangular,
	SkyBoxPanorama,
	VarianceDepthMap,
	VarianceShadow
};

/**
* Maps a string to a shader enum.
* @param str: The string to be mapped
* @return: The mapped shader enum.
*
* ATTENTION: If the string couldn't be mapped, a EnumFormatException
* will be thrown.
*/
static ShaderType stringToShaderEnum(const std::string& str);

/**
* Puts a string representation of a shader enum to an output stream.
*/
std::ostream& operator<<(std::ostream& os, ShaderType shader);

enum class UniformType
{
	CUBE_MAP,
	FLOAT,
	INT,
	MAT3,
	MAT4,
	TEXTURE2D,
	TEXTURE2D_ARRAY,
	VEC2,
	VEC3,
	VEC4
};

struct Uniform
{
	GLint location  = -1;
	UniformType type = UniformType::INT;
};

struct UniformTex : public Uniform
{
	GLuint textureUnit = 0;
};

enum class ShaderTypeGL
{
	VERTEX = GL_VERTEX_SHADER,
	FRAGMENT = GL_FRAGMENT_SHADER,
	GEOMETRY = GL_GEOMETRY_SHADER
};

std::ostream& operator<<(std::ostream& os, ShaderTypeGL type);


/**
 * Represents a shader program for an OpenGL renderer.
 */
class ShaderProgramGL
{
public:
	/**
	* Creates a new shader program from a given vertex shader and fragment shader file.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	ShaderProgramGL(
		const std::string& vertexShaderFile, 
		const std::string& fragmentShaderFile,
		const std::string& geometryShaderFile = "", 
		const std::string& instancedVertexShaderFile = "");

	ShaderProgramGL(ShaderProgramGL&& other);
	ShaderProgramGL(const ShaderProgramGL& other) = delete;

	virtual ~ShaderProgramGL() = default;

	void bind();

	GLuint getProgramID() const;

	int getUniformLocation(const char* name);

	static GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile,
		const std::string& geometryShaderFile = "");

	static ShaderSourceFileGenerator* getSourceFileGenerator();
	
	void release();

	void setDebugName(const char* name);

	void setInt(GLint uniformID, int data);
	void setFloat(GLint uniformID, float data);

	void setVec2(GLint uniformID, const glm::vec2& data);
	void setVec3(GLint uniformID, const glm::vec3& data);
	void setVec4(GLint uniformID, const glm::vec4& data);

	void setMat3(GLint uniformID, const glm::mat3& data);
	void setMat4(GLint uniformID, const glm::mat4& data);


	void setTexture(GLint uniformID, const TextureGL* data, unsigned int textureSlot);

	/*void setTexture2D(GLuint uniformID, const TextureGL* data, unsigned int textureSlot);
	void setTexture2DArray(GLuint uniformID, const TextureGL* data, unsigned int textureSlot);
	void setCubeMap(GLuint uniformID, const CubeMapGL* data, unsigned int textureSlot);
	void setCubeMapArray(GLuint uniformID, const CubeMapGL* data, unsigned int textureSlot);*/

	void unbind();


protected:

	GLuint programID;
	bool mIsBound;
	std::string mDebugName;


	static std::string adjustLineNumbers(char* message, const ProgramDesc& desc);
	static GLuint compileShader(unsigned int type, const ProgramDesc& desc);
	static GLuint createShader(const ProgramDesc& vertexShader, const ProgramDesc& fragmentShader, const ProgramDesc* geometryShader = nullptr);

	/**
	 * @param shaderSourceFile The source file for that an unfolded version should be written for.
	 * @throws ShaderInitException - if an IO error occurs
	 */
	static void writeUnfoldedShaderContentToFile(const std::string& shaderSourceFile, const std::vector<char>& sourceCode);
};


class ShaderGL
{
public:

	ShaderGL();

	virtual ~ShaderGL();

	void bind();

	ShaderProgramGL* getProgram();

	void setProgram(ShaderProgramGL* program);

	void unbind();

	virtual void onModelMatrixUpdate(const glm::mat4& modelMatrix);

	virtual void onMaterialUpdate(const Material* material);

	// Function that should be called before render calls
	virtual void setupRenderState();

	// Reverse the state of the function setupRenderState
	// TODO
	virtual void reverseRenderState();

protected:
	ShaderProgramGL* mProgram;
};

class TransformShaderGL : public ShaderGL
{
public:
	TransformShaderGL() = default;
	virtual ~TransformShaderGL() = default;

	virtual void onTransformUpdate(const TransformData& data) = 0;
};