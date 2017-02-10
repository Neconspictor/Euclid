#pragma once
#include <shader/LampShader.hpp>
#include <shader/opengl/ShaderGL.hpp>

class LampShaderGL : public ShaderGL, public LampShader
{
public:
	/**
	* Creates a new lamp shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	LampShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	
	~LampShaderGL() override;
	
	void draw(Mesh const& mesh) override;

	void drawInstanced(Mesh const& mesh, unsigned amount) override;

	void release() override;

	void use() override;
};