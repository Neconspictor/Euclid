#pragma once
#include <nex/opengl/shader/ShaderGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class GaussianBlurHorizontalShaderGL :  public ShaderConfigGL
{
public:
	GaussianBlurHorizontalShaderGL();

	virtual ~GaussianBlurHorizontalShaderGL();

	void update(const MeshGL& mesh, const TransformData& data) override;

	void setTexture(TextureGL * tex);
	void setImageWidth(float width);
	void setImageHeight(float height);

protected:
	TextureGL* image;
	ShaderAttributeCollection::ShaderAttributeKey imageAttribute;
	float height;
	ShaderAttributeCollection::ShaderAttributeKey heightAttribute;
	glm::mat4 transform;
	float width;
	ShaderAttributeCollection::ShaderAttributeKey widthAttribute;
};

class GaussianBlurVerticalShaderGL : public ShaderConfigGL
{
public:
	GaussianBlurVerticalShaderGL();

	virtual ~GaussianBlurVerticalShaderGL();

	void update(const MeshGL& mesh, const TransformData& data) override;

	void setTexture(TextureGL * tex);
	void setImageHeight(float height);
	void setImageWidth(float width);

protected:
	TextureGL* image;
	ShaderAttributeCollection::ShaderAttributeKey imageAttribute;
	float height;
	ShaderAttributeCollection::ShaderAttributeKey heightAttribute;
	bool horizontal;
	glm::mat4 transform;
	float width;
	ShaderAttributeCollection::ShaderAttributeKey widthAttribute;
};