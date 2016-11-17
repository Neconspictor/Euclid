#ifndef ENGINE_SHADER_SHADER_MANAGER_HPP
#define ENGINE_SHADER_SHADER_MANAGER_HPP
#include <shader/Shader.hpp>
#include <string>

/**
 * A facility class for creating, receiving and storing renderer independent shader programs
 */
class ShaderManager
{
public:
	ShaderManager() {}
	virtual ~ShaderManager() {}

	/**
	 * Provides a loaded shader by its class name.
	 * Before any shader can be retrieved by this function, ShaderManager::loadShaders(const std::string& folder)
	 * has to be called on this class.
	 * NOTE: A ShaderNotFound exception is thrown, if the shader name couldn't be matched
	 * to a shader.
	 */
	virtual Shader* getShader(const std::string& shaderName) = 0;

	/**
	 * Loads all shaders from a given folder, and creates shader programs from them.
	 * NOTE: 
	 * - A FolderNotExistsException is thrown, if the specified string isn't a valid folder path.
	 * - A ShaderInitException is thrown if one shader program couldn't be created.
	 */
	virtual void loadShaders(const std::string& folder) = 0;
};

#endif