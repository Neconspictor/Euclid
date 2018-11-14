#pragma once
#include <string>
#include <nex/logging/LoggingClient.hpp>
#include <glad/glad.h>
#include <unordered_map>
#include <nex/exception/ShaderInitException.hpp>

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
enum class Shaders
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
static Shaders stringToShaderEnum(const std::string& str);

/**
* Puts a string representation of a shader enum to an output stream.
*/
std::ostream& operator<<(std::ostream& os, Shaders shader);

enum class ShaderAttributeType
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


class ShaderAttributeGL
{
public:
	ShaderAttributeGL();
	ShaderAttributeGL(ShaderAttributeType type, const void* data, std::string uniformName, bool active = false);

	ShaderAttributeGL(const ShaderAttributeGL& o);
	ShaderAttributeGL& operator=(const ShaderAttributeGL& o);

	virtual ~ShaderAttributeGL();

	void activate(bool active);

	const void* getData() const;
	ShaderAttributeType getType() const;

	bool isActive() const;


	const std::string& getName() const;

	void setData(const void* data);
	void setName(std::string name);
	void setType(ShaderAttributeType type);

protected:
	std::string uniformName;
	const void* data;
	bool m_isActive;
	ShaderAttributeType type;
};

class ShaderAttributeCollection
{
public:

	using ShaderAttributeKey = int;
	using ShaderAttributeGLIterator = std::vector<ShaderAttributeGL>::iterator;

	ShaderAttributeCollection();
	ShaderAttributeCollection(const ShaderAttributeCollection& o);
	ShaderAttributeCollection& operator=(const ShaderAttributeCollection& o);

	virtual ~ShaderAttributeCollection();

	ShaderAttributeKey create(ShaderAttributeType type, const void* data, std::string uniformName, bool active = false);
	ShaderAttributeGL* get(const std::string& uniformName);
	ShaderAttributeGL* get(ShaderAttributeKey key);
	const ShaderAttributeGL* getList() const;
	void setData(const std::string& uniformName, const void* data, const void* defaultValue = nullptr, bool activate = true);

	int size() const;
protected:
	std::vector<ShaderAttributeGL> vec;
	std::unordered_map<std::string, int>  lookup;
};

class ShaderConfigGL
{
public:
	ShaderConfigGL();
	virtual ~ShaderConfigGL();

	virtual void afterDrawing(const MeshGL& mesh);

	virtual void beforeDrawing(const MeshGL& mesh);

	virtual const ShaderAttributeGL* getAttributeList() const;

	virtual int getNumberOfAttributes() const;

	virtual void update(const MeshGL& mesh, const TransformData& data) = 0;
protected:
	ShaderAttributeCollection attributes;
};



/**
 * Represents a shader program for an OpenGL renderer.
 */
class ShaderGL
{
public:
	/**
	* Creates a new shader program from a given vertex shader and fragment shader file.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	ShaderGL(std::unique_ptr<ShaderConfigGL> config, 
		const std::string& vertexShaderFile, 
		const std::string& fragmentShaderFile,
		const std::string& geometryShaderFile = "", 
		const std::string& instancedVertexShaderFile = "");

	/**
	 * Creates a new shader with now shader configuration object
	 */
	ShaderGL(const std::string& vertexShaderFile,
		const std::string& fragmentShaderFile,
		const std::string& geometryShaderFile = "",
		const std::string& instancedVertexShaderFile = "");

	ShaderGL(ShaderGL&& other);
	ShaderGL(const ShaderGL& other) = delete;

	virtual ~ShaderGL();

	static bool compileShaderComponent(const std::string& shaderContent, GLuint shaderResourceID);

	virtual void draw(MeshGL const& mesh);

	virtual void drawInstanced(MeshGL const& mesh, unsigned amount);

	ShaderConfigGL* getConfig() const;

	void setTransformData(TransformData data);

	GLuint getProgramID() const;
	
	static GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile, 
		const std::string& geometryShaderFile = "");
	
	virtual void release();
	
	virtual void use();

	static void initShaderFileSystem();

protected:
	std::unique_ptr<ShaderConfigGL> config;
	GLuint programID;
	GLuint instancedProgramID;
	nex::LoggingClient logClient;
	GLint textureCounter;

	TransformData data;

	/**
	* Extracts an #include statements from a line.
	* If the line contains no #include directive, false will be returned.
	*/
	static bool extractIncludeStatement(const std::string& line, std::string* result);

	/**
	 *@param shaderFile The shader file which is expected to be a valid glsl shader file
	 *@param writeUnfoldedResult Should the unfolded shader content (resolved include statements) be written to an *.unfolded file?
	 *@param defines - An optional list of preprocessor define directives
	 *
	 * @return The loaded (and unfolded) shader content
	 *
	 * @throws ShaderInitException - if the shader couldn't be loaded
	 */
	static std::string loadShaderComponent(const std::string& shaderFile, bool writeUnfoldedResult = true, std::vector<std::string> defines = {});

	/**
	* Preprocess a glsl shader.
	* A list of given defines will be added to the shader and #include statements in the shader will be replaced by
	* the content of the respective include file.
	*
	* The Output is a vector of trimmed and comment-free lines
	*
	* @throws ShaderInitException - if one of the include files couldn't be loaded
	*/
	static std::vector<std::string> preprocess(std::string& shaderContent, const std::vector<std::string>& defines);

	/**
	 * @param shaderSourceFile The source file for that an unfolded version should be written for.
	 * @throws ShaderInitException - if an IO error occurs
	 */
	static void writeUnfoldedShaderContentToFile(const std::string& shaderSourceFile, const std::vector<std::string>& lines);

	virtual void afterDrawing(const MeshGL& mesh);
	virtual void beforeDrawing(const MeshGL& mesh);

	virtual void setAttribute(GLuint program, const ShaderAttributeGL& attribute);
};