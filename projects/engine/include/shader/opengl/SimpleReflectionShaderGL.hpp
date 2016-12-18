#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <texture/opengl/CubeMapGL.hpp>
#include <shader/SimpleReflectionShader.hpp>

class SimpleReflectionShaderGL : public ShaderGL, public SimpleReflectionShader
{
public:
	/**
	* Creates a new simple reflection shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	SimpleReflectionShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	~SimpleReflectionShaderGL() override;

	void draw(Mesh const& mesh) override;

	void release() override;

	void setCameraPosition(glm::vec3 position) override;

	void setReflectionTexture(CubeMap* reflectionTex) override;

	void use() override;

private:
	CubeMapGL* reflectionTexture;
	glm::vec3 cameraPosition;
};
