#pragma once
#include <shader/SkyBoxShader.hpp>
#include <shader/opengl/ShaderGL.hpp>
#include <texture/opengl/TextureGL.hpp>

class SkyBoxShaderGL : public SkyBoxShader, public ShaderConfigGL
{
public:
	SkyBoxShaderGL();

	virtual ~SkyBoxShaderGL() override;

	void afterDrawing() override;

	void beforeDrawing() override;

	virtual void setSkyTexture(CubeMap* sky) override;

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	CubeMapGL* skyTexture;
	glm::mat4 transform;
};

class PanoramaSkyBoxShaderGL : public PanoramaSkyBoxShader, public ShaderConfigGL
{
public:
	PanoramaSkyBoxShaderGL();

	~PanoramaSkyBoxShaderGL() override;

	void afterDrawing() override;

	void beforeDrawing() override;

	virtual void setSkyTexture(Texture* tex) override;

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	TextureGL* skyTexture;
	glm::mat4 transform;
};