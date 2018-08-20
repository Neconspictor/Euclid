#pragma once
#include <nex/shader/SkyBoxShader.hpp>
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class SkyBoxShaderGL : public SkyBoxShader, public ShaderConfigGL
{
public:
	SkyBoxShaderGL();

	virtual ~SkyBoxShaderGL() override;

	void afterDrawing(const MeshGL& mesh) override;

	void beforeDrawing(const MeshGL& mesh) override;

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

	void afterDrawing(const MeshGL& mesh) override;

	void beforeDrawing(const MeshGL& mesh) override;

	virtual void setSkyTexture(Texture* tex) override;

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	TextureGL* skyTexture;
	glm::mat4 transform;
};

class EquirectangularSkyBoxShaderGL : public EquirectangularSkyBoxShader, public ShaderConfigGL
{
public:
	EquirectangularSkyBoxShaderGL();

	~EquirectangularSkyBoxShaderGL() override;

	void afterDrawing(const MeshGL& mesh) override;

	void beforeDrawing(const MeshGL& mesh) override;

	virtual void setSkyTexture(Texture* tex) override;

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	TextureGL* skyTexture;
};