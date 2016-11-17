#ifndef ENGINE_SHADER_OPENGL_SHADER_MANAGERGL_HPP
#define ENGINE_SHADER_OPENGL_SHADER_MANAGERGL_HPP
#include <shader/ShaderManager.hpp>
#include <map>
#include <memory>
#include "ShaderGL.hpp"

class ShaderManagerGL : public ShaderManager
{
public:
	virtual ~ShaderManagerGL() override;
	virtual Shader* getShader(const std::string& shaderName) override;
	virtual void loadShaders(const std::string& folder) override;
private:
	std::map<std::string, std::unique_ptr<ShaderGL>> shaderMap;
};
#endif