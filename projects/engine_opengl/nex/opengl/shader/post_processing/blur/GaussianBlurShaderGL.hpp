#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class GaussianBlurHorizontalShaderGL : public TransformShaderGL
{
public:
	GaussianBlurHorizontalShaderGL();

	virtual ~GaussianBlurHorizontalShaderGL() = default;

	void setImageHeight(float height);
	void setImageWidth(float width);

	void setTexture(const TextureGL* tex);

	void setMVP(const glm::mat4& mvp);


	void onTransformUpdate(const TransformData& data) override;
protected:
	UniformTex image;
	Uniform transform;
	Uniform windowWidth;
	Uniform windowHeight;
};

class GaussianBlurVerticalShaderGL : public TransformShaderGL
{
public:
	GaussianBlurVerticalShaderGL();

	virtual ~GaussianBlurVerticalShaderGL() = default;

	void setImageHeight(float height);
	void setImageWidth(float width);

	void setTexture(const TextureGL* tex);

	void setMVP(const glm::mat4& mvp);

	void onTransformUpdate(const TransformData& data) override;
protected:
	UniformTex image;
	Uniform transform;
	Uniform windowWidth;
	Uniform windowHeight;
};