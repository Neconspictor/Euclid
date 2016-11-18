#ifndef ENGINE_SHADER_SHADER_MANAGER_HPP
#define ENGINE_SHADER_SHADER_MANAGER_HPP
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
};

#endif