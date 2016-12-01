#ifndef ENGINE_SHADER_OPENGL_SHADER_MANAGERGL_HPP
#define ENGINE_SHADER_OPENGL_SHADER_MANAGERGL_HPP
#include <shader/ShaderManager.hpp>
#include <map>
#include <memory>
#include <shader/opengl/ShaderGL.hpp>

/**
 * An opengl implementation of a shader manager
 */
class ShaderManagerGL : public ShaderManager
{
public:
	virtual ~ShaderManagerGL() override;
	virtual Shader* getShader(ShaderEnum shader) override;
	virtual void loadShaders() override;

	/**
	* Provides access the shader manager singleton.
	*/
	static ShaderManagerGL* get();

private:
	std::map<ShaderEnum, std::shared_ptr<ShaderGL>> shaderMap;
	platform::LoggingClient logClient;

	// this class is a singleton, thus private constructor
	ShaderManagerGL();

	/**
	 * Creates a shader by its enum and registers it to the shader map.
	 * @return : A pointer to the created shader.
	 *
	 * NOTE: A ShaderInitException will be thrown if the shader can't be created.
	 */
	Shader* createShader(ShaderEnum shaderEnum);

	static std::unique_ptr<ShaderManagerGL> instance;
};
#endif