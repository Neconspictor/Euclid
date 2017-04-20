#pragma once
#include <shader/opengl/ShaderGL.hpp>
#include <shader/post_processing/blur/GaussianBlurShader.hpp>
#include <texture\opengl\TextureGL.hpp>

class GaussianBlurHorizontalShaderGL : public GaussianBlurHorizontalShader, public ShaderConfigGL
{
public:
	GaussianBlurHorizontalShaderGL();

	virtual ~GaussianBlurHorizontalShaderGL();

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

	// Inherited via GaussianBlurShader
	virtual void setTexture(Texture * tex) override;
	virtual void setImageWidth(float width) override;

protected:
	TextureGL* image;
	ShaderAttributeCollection::ShaderAttributeKey imageAttribute;
	glm::mat4 transform;
	float width;
	ShaderAttributeCollection::ShaderAttributeKey widthAttribute;
};

class GaussianBlurVerticalShaderGL : public GaussianBlurVerticalShader, public ShaderConfigGL
{
public:
	GaussianBlurVerticalShaderGL();

	virtual ~GaussianBlurVerticalShaderGL();

	virtual void update(const MeshGL& mesh, const TransformData& data) override;

	// Inherited via GaussianBlurShader
	virtual void setTexture(Texture * tex) override;
	virtual void setImageHeight(float height) override;

protected:
	TextureGL* image;
	ShaderAttributeCollection::ShaderAttributeKey imageAttribute;
	float height;
	ShaderAttributeCollection::ShaderAttributeKey heightAttribute;
	bool horizontal;
	glm::mat4 transform;
};