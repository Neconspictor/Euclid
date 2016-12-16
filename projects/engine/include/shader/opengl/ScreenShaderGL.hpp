#pragma once
#include <shader/ScreenShader.hpp>
#include <shader/opengl/ShaderGL.hpp>

class ScreenShaderGL : public ScreenShader, ShaderGL
{
public:
	/**
	* Creates a new screen shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	ScreenShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	virtual ~ScreenShaderGL();

	void draw(Mesh const& mesh) override;
	void release() override;

	void setOffscreenBuffer(GLuint frameBuffer);

	void use() override;

protected:
	GLuint frameBuffer;
};