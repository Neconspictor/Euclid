#pragma once
#include <shader/Shader.hpp>
#include <shader/ShaderEnum.hpp>

/**
 * An interface for creating, receiving and storing renderer independent shaders
 */
class ShaderManager
{
public:
	ShaderManager() {}
	virtual ~ShaderManager() {}

	/**
	 * Provides a singleton of a shader by its shader enumeration.
	 * NOTE: A ShaderInitException can be thrown if the specified has to be created but 
	 * an error occured during initialization.
	 */
	virtual Shader* getShader(ShaderEnum shader) = 0;

	/**
	 * Loads all shaders.
	 * NOTE: A ShaderInitException is thrown if one shaders couldn't be created.
	 */
	virtual void loadShaders() = 0;

	/**
	* Checks, if the specified shader is an implementation of the underlying render engine
	* NOTE: A runtime error is thrown if the validation fails!
	*/
	virtual void validateShader(Shader* shader) = 0;
};