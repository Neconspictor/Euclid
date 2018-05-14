#pragma once
#include <shader/ShaderGL.hpp>
#include <shader/DepthMapShader.hpp>
#include <texture/TextureGL.hpp>

class CubeDepthMapShaderGL : public CubeDepthMapShader, public ShaderConfigGL
{
public:
	CubeDepthMapShaderGL();

	virtual ~CubeDepthMapShaderGL();
	
	void useCubeDepthMap(CubeMap* map) override;

	void setLightPos(glm::vec3 pos) override;

	void setRange(float range) override;

	void update(const MeshGL& mesh, const TransformData& data) override;

private:
	CubeMapGL* cubeMap;
	glm::vec3 lightPos;
	float range;
	glm::mat4 transform;
};

class DepthMapShaderGL : public DepthMapShader, public ShaderConfigGL
{
public:
	DepthMapShaderGL();

	virtual ~DepthMapShaderGL();

	void update(const MeshGL& mesh, const TransformData& data) override;

	void useDepthMapTexture(Texture* texture) override;

private:
	TextureGL* texture;
	glm::mat4 transform;
};

class VarianceDepthMapShaderGL : public VarianceDepthMapShader, public ShaderConfigGL
{
public:
	VarianceDepthMapShaderGL();

	virtual ~VarianceDepthMapShaderGL();

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

	virtual void useVDepthMapTexture(Texture* texture) override;

private:
	TextureGL* texture;
	glm::mat4 transform;
};