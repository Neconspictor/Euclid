#pragma once
#include <shader/ScreenShader.hpp>
#include <shader/opengl/ShaderGL.hpp>
#include <texture/opengl/TextureGL.hpp>

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
	void drawInstanced(Mesh const& mesh, unsigned amount) override;
	void release() override;

	void use() override;

	void useTexture(Texture* texture) override;

protected:
	TextureGL* texture;
};
