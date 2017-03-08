#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/DepthMapShader.hpp>
#include <texture/opengl/TextureGL.hpp>

class CubeDepthMapShaderGL : public CubeDepthMapShader
{
public:
	CubeDepthMapShaderGL();

	virtual ~CubeDepthMapShaderGL();

	const ShaderAttribute* getAttributeList() const override;

	int getNumberOfAttributes() const override;
	
	void useCubeDepthMap(CubeMap* map) override;

	void setLightPos(glm::vec3 pos) override;

	void setRange(float range) override;

	void update(const TransformData& data) override;

private:
	ShaderAttributeCollection attributes;
	CubeMapGL* cubeMap;
	glm::vec3 lightPos;
	glm::mat4 model;
	float range;
	glm::mat4 transform;
};

class DepthMapShaderGL : public DepthMapShader
{
public:

	DepthMapShaderGL();

	virtual ~DepthMapShaderGL();

	void useDepthMapTexture(Texture* texture) override;

private:
	ShaderAttributeCollection attributes;
	TextureGL* texture;
};