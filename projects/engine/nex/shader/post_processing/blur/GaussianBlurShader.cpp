#include <nex/shader/post_processing/blur/GaussianBlurShader.hpp>
#include "nex/texture/TextureManager.hpp"

using namespace glm;
using namespace nex;

//height(1024), width(800)

GaussianBlurHorizontalShader::GaussianBlurHorizontalShader()
{
	mProgram = ShaderProgram::create(
		"post_processing/blur/gaussian_blur_vs.glsl", "post_processing/blur/gaussian_blur_horizontal_fs.glsl");

	image			= { mProgram->getUniformLocation("image"), UniformType::TEXTURE2D, 0};
	transform		= { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	windowWidth		= { mProgram->getUniformLocation("windowWidth"), UniformType::FLOAT };
	windowHeight	= { mProgram->getUniformLocation("windowHeight"), UniformType::FLOAT };

	mProgram->setBinding(image.location, image.bindingSlot);
}


void GaussianBlurHorizontalShader::setTexture(const Texture* tex)
{
	mProgram->setTexture(tex, &mSampler, image.bindingSlot);
}

void GaussianBlurHorizontalShader::setMVP(const mat4& mvp)
{
	mProgram->setMat4(transform.location, mvp);
}

void GaussianBlurHorizontalShader::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection)*(*data.view)*(*data.model));
}

void GaussianBlurHorizontalShader::setImageWidth(float width)
{
	mProgram->setFloat(windowWidth.location, width);
}

void GaussianBlurHorizontalShader::setImageHeight(float height)
{
	mProgram->setFloat(windowHeight.location, height);
}


//height(1024), width(1024)

GaussianBlurVerticalShader::GaussianBlurVerticalShader()
{
	mProgram = ShaderProgram::create(
		"post_processing/blur/gaussian_blur_vs.glsl", "post_processing/blur/gaussian_blur_vertical_fs.glsl");

	image = { mProgram->getUniformLocation("image"), UniformType::TEXTURE2D, 0 };
	transform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	windowWidth = { mProgram->getUniformLocation("windowWidth"), UniformType::FLOAT };
	windowHeight = { mProgram->getUniformLocation("windowHeight"), UniformType::FLOAT };

	mProgram->setBinding(image.location, image.bindingSlot);
}


void GaussianBlurVerticalShader::setTexture(const Texture* tex)
{
	mProgram->setTexture(tex, &mSampler, image.bindingSlot);
}

void GaussianBlurVerticalShader::setMVP(const mat4& mvp)
{
	mProgram->setMat4(transform.location, mvp);
}

void GaussianBlurVerticalShader::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection)*(*data.view)*(*data.model));
}

void GaussianBlurVerticalShader::setImageWidth(float width)
{
	mProgram->setFloat(windowWidth.location, width);
}

void GaussianBlurVerticalShader::setImageHeight(float height)
{
	mProgram->setFloat(windowHeight.location, height);
}