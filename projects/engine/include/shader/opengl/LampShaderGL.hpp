#ifndef ENGINE_SHADER_OPENGL_LAMPSHADER_HPP
#define ENGINE_SHADER_OPENGL_LAMPSHADER_HPP
#include <shader/LampShader.hpp>
#include <shader/opengl/ShaderGL.hpp>
#include <model/Model.hpp>

class LampShaderGL : public ShaderGL, public LampShader
{
public:
	/**
	* Creates a new lamp shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	LampShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	
	~LampShaderGL() override;
	
	void draw(Model const& model, glm::mat4 const& transform) override;

	bool loadingFailed() override;

	void release() override;

	void use() override;
};
#endif