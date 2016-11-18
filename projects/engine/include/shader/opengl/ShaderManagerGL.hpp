#ifndef ENGINE_SHADER_OPENGL_SHADER_MANAGERGL_HPP
#define ENGINE_SHADER_OPENGL_SHADER_MANAGERGL_HPP
#include <shader/ShaderManager.hpp>
#include <map>
#include <memory>
#include <shader/opengl/ShaderGL.hpp>

class ShaderManagerGL : public ShaderManager
{
public:
	ShaderManagerGL();
	virtual ~ShaderManagerGL() override;
	virtual Shader* getShader(ShaderEnum shader) override;
	virtual void loadShaders() override;
private:
	std::map<ShaderEnum, std::unique_ptr<ShaderGL>> shaderMap;
	platform::LoggingClient logClient;

	/**
	 * Creates a shader by its enum and registers it to the shader map.
	 * @return : A pointer to the created shader.
	 *
	 * NOTE: A ShaderInitException will be thrown if the shader can't be created.
	 */
	Shader* createShader(ShaderEnum shaderEnum);
};
#endif