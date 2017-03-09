#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/ShadowShader.hpp>

class PointShadowShaderGL : public PointShadowShader, public ShaderConfigGL
{
public:
	/**
	* Creates a new shadow shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	PointShadowShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile,
		const std::string& geometryShaderFile);

	virtual ~PointShadowShaderGL();

	void draw(Mesh const& mesh) override;

	void drawInstanced(Mesh const& mesh, unsigned amount) override;

	void release() override;

	void setLightPosition(glm::vec3 pos) override;

	void setRange(float range) override;

	void setShadowMatrices(glm::mat4 matrices[6]) override;

	void use() override;

	glm::vec3 lightPos;
	glm::mat4* matrices;
	float range;
};

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

class VarianceShadowShaderGL : public ShaderGL, public VarianceShadowShader
{
public:
	/**
	* Creates a new shadow shader program.
	* NOTE: If an error occurs while creating the shader program, a ShaderInitException will be thrown!
	*/
	VarianceShadowShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	virtual ~VarianceShadowShaderGL();

	void draw(Mesh const& mesh) override;

	void drawInstanced(Mesh const& mesh, unsigned amount) override;

	void release() override;

	void use() override;
};