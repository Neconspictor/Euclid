#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/DepthMapShader.hpp>
#include <texture/opengl/TextureGL.hpp>

class DepthMapShaderGL : public ShaderGL, public DepthMapShader
{
public:
	/**
	* Creates a new depth map shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	DepthMapShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	virtual ~DepthMapShaderGL();

	void draw(Mesh const& mesh) override;

	void drawInstanced(Mesh const& mesh, unsigned amount) override;

	void release() override;

	void use() override;

	void useDepthMapTexture(Texture* texture) override;

private:
	TextureGL* texture;
};
