#pragma once
#include <shader/SkyBoxShader.hpp>
#include <shader/opengl/ShaderGL.hpp>
#include <texture/opengl/CubeMapGL.hpp>

class SkyBoxShaderGL : public ShaderGL, public SkyBoxShader
{
public:
	/**
	* Creates a new lamp shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	SkyBoxShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	~SkyBoxShaderGL() override;

	void draw(Mesh const& mesh) override;

	void drawInstanced(Mesh const& mesh, unsigned amount) override;

	void release() override;

	void setSkyTexture(CubeMap* sky) override;

	void use() override;

private:
	CubeMapGL* skyTexture;
};
