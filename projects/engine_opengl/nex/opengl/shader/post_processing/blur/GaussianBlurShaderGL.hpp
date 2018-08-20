#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/shader/post_processing/blur/GaussianBlurShader.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

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
	float height;
	ShaderAttributeCollection::ShaderAttributeKey heightAttribute;
	glm::mat4 transform;
	float width;
	ShaderAttributeCollection::ShaderAttributeKey widthAttribute;

	// Inherited via GaussianBlurHorizontalShader
	virtual void setImageHeight(float height) override;
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
	float width;
	ShaderAttributeCollection::ShaderAttributeKey widthAttribute;

	// Inherited via GaussianBlurVerticalShader
	virtual void setImageWidth(float width) override;
};