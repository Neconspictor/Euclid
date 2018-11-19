#pragma once
#include <map>
#include <memory>
#include <nex/opengl/shader/ShaderGL.hpp>

/**
 * An opengl implementation of a shader manager
 */
 /**
  * An interface for creating, receiving and storing renderer independent shaders
  */
class ShaderManagerGL
{
public:
	virtual ~ShaderManagerGL();

	/**
	 * Provides a singleton of a shader by its shader enumeration.
	 * NOTE: A ShaderInitException can be thrown if the specified has to be created but
	 * an error occured during initialization.
	 */
	ShaderGL* getShader(ShaderType shader);

	/**
	 * Loads all shaders.
	 * NOTE: A ShaderInitException is thrown if one shaders couldn't be created.
	 */
	void loadShaders();

	/**
	* Checks, if the specified shader is an implementation of the underlying render engine
	* NOTE: A runtime error is thrown if the validation fails!
	*/
	void validateShader(ShaderProgramGL* shader);
	/**
	* Provides access the shader manager singleton.
	*/
	static ShaderManagerGL* get();

private:
	std::map<ShaderType, std::unique_ptr<ShaderGL>> shaderMap;
	nex::Logger m_logger;

	// this class is a singleton, thus private constructor
	ShaderManagerGL();

	/**
	 * Creates a shader by its enum and registers it to the shader map.
	 * @return : A pointer to the created shader.
	 *
	 * NOTE: A ShaderInitException will be thrown if the shader can't be created.
	 */
	ShaderGL* createShader(ShaderType shaderEnum);

	static std::unique_ptr<ShaderManagerGL> instance;
};