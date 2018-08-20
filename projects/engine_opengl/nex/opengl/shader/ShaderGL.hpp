#pragma once
#include <string>
#include <nex/logging/LoggingClient.hpp>
#include <glad/glad.h>
#include <nex/shader/Shader.hpp>
#include <unordered_map>

class MeshGL;
class Vob;

class ShaderAttributeGL : public ShaderAttribute
{
public:
	ShaderAttributeGL();
	ShaderAttributeGL(const ShaderAttributeGL& o);
	ShaderAttributeGL(ShaderAttributeGL&& o);
	ShaderAttributeGL& operator=(const ShaderAttributeGL& o);
	ShaderAttributeGL&& operator=(ShaderAttributeGL&& o);
	

	ShaderAttributeGL(ShaderAttributeType type, const void* data, std::string uniformName, bool active = false);
	virtual ~ShaderAttributeGL() override;

	const std::string& getName() const;

	void setData(const void* data);
	void setName(std::string name);
	void setType(ShaderAttributeType type);

protected:
	std::string uniformName;
};

class ShaderAttributeCollection
{
public:

	using ShaderAttributeKey = int;
	using ShaderAttributeGLIterator = std::vector<ShaderAttributeGL>::iterator;

	ShaderAttributeCollection();
	ShaderAttributeCollection(const ShaderAttributeCollection& o);
	ShaderAttributeCollection(ShaderAttributeCollection&& o);
	ShaderAttributeCollection& operator=(const ShaderAttributeCollection& o);
	ShaderAttributeCollection&& operator=(ShaderAttributeCollection&& o);
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

class ShaderConfigGL : public ShaderConfig
{
public:
	ShaderConfigGL();
	virtual ~ShaderConfigGL();

	virtual void afterDrawing(const MeshGL& mesh);

	virtual void beforeDrawing(const MeshGL& mesh);

	virtual const ShaderAttribute* getAttributeList() const;

	virtual int getNumberOfAttributes() const;

	virtual void update(const MeshGL& mesh, const TransformData& data) = 0;
protected:
	ShaderAttributeCollection attributes;
};

/**
 * Represents a shader program for an OpenGL renderer.
 */
class ShaderGL : public Shader
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

	static bool compileShader(const std::string& shaderContent, GLuint shaderResourceID);

	void draw(Mesh const& mesh) override;

	void drawInstanced(Mesh const& mesh, unsigned amount) override;

	virtual ShaderConfig* getConfig() const override;

	GLuint getProgramID() const;
	
	static GLuint loadShaders(const std::string& vertexFile, const std::string& fragmentFile, 
		const std::string& geometryShaderFile = "");
	
	virtual void release() override;
	
	virtual void use();

	static void initShaderFileSystem();

protected:
	std::unique_ptr<ShaderConfigGL> config;
	GLuint programID;
	GLuint instancedProgramID;
	nex::LoggingClient logClient;
	GLint textureCounter;

	virtual void afterDrawing(const MeshGL& mesh);
	virtual void beforeDrawing(const MeshGL& mesh);

	virtual void setAttribute(GLuint program, const ShaderAttributeGL& attribute);
};