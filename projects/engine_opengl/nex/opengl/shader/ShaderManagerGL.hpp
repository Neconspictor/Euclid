#pragma once
#include <nex/shader/ShaderManager.hpp>
#include <map>
#include <memory>
#include <nex/opengl/shader/ShaderGL.hpp>

/**
 * An opengl implementation of a shader manager
 */
class ShaderManagerGL : public ShaderManager
{
public:
	virtual ~ShaderManagerGL() override;
	virtual ShaderConfig* getConfig(Shaders shader) override;
	virtual Shader* getShader(Shaders shader) override;
	virtual void loadShaders() override;
	virtual void validateShader(Shader* shader) override;
	/**
	* Provides access the shader manager singleton.
	*/
	static ShaderManagerGL* get();

private:
	std::map<Shaders, std::shared_ptr<Shader>> shaderMap;
	nex::LoggingClient logClient;

	// this class is a singleton, thus private constructor
	ShaderManagerGL();

	/**
	 * Creates a shader by its enum and registers it to the shader map.
	 * @return : A pointer to the created shader.
	 *
	 * NOTE: A ShaderInitException will be thrown if the shader can't be created.
	 */
	Shader* createShader(Shaders shaderEnum);

	static std::unique_ptr<ShaderManagerGL> instance;
};