#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/ShadowShader.hpp>

class ShadowShaderGL : public ShaderGL, public ShadowShader
{
public:
	/**
	* Creates a new shadow shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	ShadowShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	virtual ~ShadowShaderGL();

	void draw(Mesh const& mesh) override;

	void drawInstanced(Mesh const& mesh, unsigned amount) override;

	void release() override;

	void use() override;
};