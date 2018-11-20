#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class SkyBoxShaderGL : public ShaderGL
{
public:
	SkyBoxShaderGL();

	virtual ~SkyBoxShaderGL() = default;

	void setMVP(const glm::mat4& mat);
	void setProjection(const glm::mat4& mat);
	void setView(const glm::mat4& mat);

	void setSkyTexture(const CubeMapGL* sky);

	void setupRenderState() override;

	void reverseRenderState() override;

private:

	UniformTex mSkyTexture;
	Uniform mTransform;
	Uniform mProjection;
	Uniform mView;
};

class PanoramaSkyBoxShaderGL : public ShaderGL
{
public:
	PanoramaSkyBoxShaderGL();

	virtual ~PanoramaSkyBoxShaderGL() = default;

	void setProjection(const glm::mat4& mat);
	void setView(const glm::mat4& mat);
	void setSkyTexture(const TextureGL* tex);

private:

	UniformTex mSkyTexture;
	Uniform mProjection;
	Uniform mView;
};

class EquirectangularSkyBoxShaderGL : public ShaderGL
{
public:
	EquirectangularSkyBoxShaderGL();

	virtual ~EquirectangularSkyBoxShaderGL() = default;

	void setProjection(const glm::mat4& mat);
	void setView(const glm::mat4& mat);
	void setSkyTexture(const TextureGL* texture);

private:
	UniformTex mSkyTexture;
	Uniform mProjection;
	Uniform mView;
};