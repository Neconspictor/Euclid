#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class GaussianBlurHorizontalShaderGL : public ShaderGL
{
public:
	GaussianBlurHorizontalShaderGL();

	virtual ~GaussianBlurHorizontalShaderGL() = default;

	void setImageHeight(float height);
	void setImageWidth(float width);

	void setTexture(TextureGL * tex);

	void setMVP(const glm::mat4& mvp);
	

protected:
	UniformTex image;
	Uniform transform;
	Uniform windowWidth;
	Uniform windowHeight;
};

class GaussianBlurVerticalShaderGL : public ShaderGL
{
public:
	GaussianBlurVerticalShaderGL();

	virtual ~GaussianBlurVerticalShaderGL() = default;

	void setImageHeight(float height);
	void setImageWidth(float width);

	void setTexture(TextureGL * tex);

	void setMVP(const glm::mat4& mvp);

protected:
	UniformTex image;
	Uniform transform;
	Uniform windowWidth;
	Uniform windowHeight;
};