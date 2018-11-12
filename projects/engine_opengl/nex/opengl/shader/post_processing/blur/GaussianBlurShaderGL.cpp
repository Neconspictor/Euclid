#include <nex/opengl/shader/post_processing/blur/GaussianBlurShaderGL.hpp>

using namespace glm;

GaussianBlurHorizontalShaderGL::GaussianBlurHorizontalShaderGL() : ShaderConfigGL(), image(nullptr), height(1024), width(800)
{
	imageAttribute = attributes.create(ShaderAttributeType::TEXTURE2D, nullptr, "image");
	widthAttribute = attributes.create(ShaderAttributeType::FLOAT, &width, "windowWidth", true);
	heightAttribute = attributes.create(ShaderAttributeType::FLOAT, &height, "windowHeight", true);
	attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
}

GaussianBlurHorizontalShaderGL::~GaussianBlurHorizontalShaderGL()
{
	image = nullptr;
}


void GaussianBlurHorizontalShaderGL::setTexture(TextureGL * tex)
{
	image = dynamic_cast<TextureGL*>(tex);
	assert(image != nullptr);
	attributes.get(imageAttribute)->setData(image);
	attributes.get(imageAttribute)->activate(true);
}

void GaussianBlurHorizontalShaderGL::setImageWidth(float width)
{
	this->width = width;
}

void GaussianBlurHorizontalShaderGL::setImageHeight(float height)
{
	this->height = height;
}

void GaussianBlurHorizontalShaderGL::update(const MeshGL & mesh, const TransformData & data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;
}

GaussianBlurVerticalShaderGL::GaussianBlurVerticalShaderGL() : ShaderConfigGL(), image(nullptr), height(1024), width(1024)
{
	imageAttribute = attributes.create(ShaderAttributeType::TEXTURE2D, nullptr, "image");
	heightAttribute = attributes.create(ShaderAttributeType::FLOAT, &height, "windowHeight", true);
	widthAttribute = attributes.create(ShaderAttributeType::FLOAT, &width, "windowWidth", true);
	attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
}

GaussianBlurVerticalShaderGL::~GaussianBlurVerticalShaderGL()
{
	image = nullptr;
}


void GaussianBlurVerticalShaderGL::setTexture(TextureGL * tex)
{
	image = dynamic_cast<TextureGL*>(tex);
	assert(image != nullptr);
	attributes.get(imageAttribute)->setData(image);
	attributes.get(imageAttribute)->activate(true);
}

void GaussianBlurVerticalShaderGL::setImageHeight(float height)
{
	this->height = height;
}

void GaussianBlurVerticalShaderGL::setImageWidth(float width)
{
	this->width = width;
}

void GaussianBlurVerticalShaderGL::update(const MeshGL & mesh, const TransformData & data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;
}