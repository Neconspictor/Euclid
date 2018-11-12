#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class SkyBoxShaderGL : public ShaderConfigGL
{
public:
	SkyBoxShaderGL();

	virtual ~SkyBoxShaderGL();

	void afterDrawing(const MeshGL& mesh) override;

	void beforeDrawing(const MeshGL& mesh) override;

	void setSkyTexture(CubeMapGL* sky);

	void update(const MeshGL& mesh, const TransformData& data) override;

private:
	CubeMapGL* skyTexture;
	glm::mat4 transform;
};

class PanoramaSkyBoxShaderGL : public ShaderConfigGL
{
public:
	PanoramaSkyBoxShaderGL();

	virtual ~PanoramaSkyBoxShaderGL();

	void afterDrawing(const MeshGL& mesh) override;

	void beforeDrawing(const MeshGL& mesh) override;

	void setSkyTexture(TextureGL* tex);

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

private:
	TextureGL* skyTexture;
	glm::mat4 transform;
};

class EquirectangularSkyBoxShaderGL : public ShaderConfigGL
{
public:
	EquirectangularSkyBoxShaderGL();

	virtual ~EquirectangularSkyBoxShaderGL();

	void afterDrawing(const MeshGL& mesh) override;

	void beforeDrawing(const MeshGL& mesh) override;

	void setSkyTexture(TextureGL* tex);

	void update(const MeshGL& mesh, const TransformData& data) override;

private:
	TextureGL* skyTexture;
};