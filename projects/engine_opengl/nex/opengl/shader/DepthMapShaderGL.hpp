#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class CubeDepthMapShaderGL : public ShaderConfigGL
{
public:
	CubeDepthMapShaderGL();

	virtual ~CubeDepthMapShaderGL();
	
	void useCubeDepthMap(CubeMapGL* map);

	void setLightPos(glm::vec3 pos);

	void setRange(float range);

	void update(const MeshGL& mesh, const TransformData& data) override;

private:
	CubeMapGL* cubeMap;
	glm::vec3 lightPos;
	float range;
	glm::mat4 transform;
};

class DepthMapShaderGL : public ShaderConfigGL
{
public:
	DepthMapShaderGL();

	virtual ~DepthMapShaderGL();

	void beforeDrawing(const MeshGL& mesh) override;

	void afterDrawing(const MeshGL& mesh) override;

	void update(const MeshGL& mesh, const TransformData& data) override;

	void useDepthMapTexture(TextureGL* texture);

private:
	TextureGL* texture;
	glm::mat4 transform;
};

class VarianceDepthMapShaderGL : public ShaderConfigGL
{
public:
	VarianceDepthMapShaderGL();

	virtual ~VarianceDepthMapShaderGL();

	void update(const MeshGL& mesh, const TransformData& data) override;

	void useVDepthMapTexture(TextureGL* texture);

private:
	TextureGL* texture;
	glm::mat4 transform;
};