#include <nex/post_processing/blur/GaussianBlurPass.hpp>
#include "nex/texture/TextureManager.hpp"


nex::GaussianBlurHorizontalShader::GaussianBlurHorizontalShader()
{
	mShader = Shader::create(
		"post_processing/blur/gaussian_blur_vs.glsl", "post_processing/blur/gaussian_blur_horizontal_fs.glsl");

	image			= { mShader->getUniformLocation("image"), UniformType::TEXTURE2D, 0};
	windowWidth		= { mShader->getUniformLocation("windowWidth"), UniformType::FLOAT };
	windowHeight	= { mShader->getUniformLocation("windowHeight"), UniformType::FLOAT };

	mShader->setBinding(image.location, image.bindingSlot);
}


void nex::GaussianBlurHorizontalShader::setTexture(const Texture* tex)
{
	mShader->setTexture(tex, &mSampler, image.bindingSlot);
}

void nex::GaussianBlurHorizontalShader::setImageWidth(float width)
{
	mShader->setFloat(windowWidth.location, width);
}

void nex::GaussianBlurHorizontalShader::setImageHeight(float height)
{
	mShader->setFloat(windowHeight.location, height);
}


//height(1024), width(1024)

nex::GaussianBlurVerticalShader::GaussianBlurVerticalShader()
{
	mShader = Shader::create(
		"post_processing/blur/gaussian_blur_vs.glsl", "post_processing/blur/gaussian_blur_vertical_fs.glsl");

	image = { mShader->getUniformLocation("image"), UniformType::TEXTURE2D, 0 };
	windowWidth = { mShader->getUniformLocation("windowWidth"), UniformType::FLOAT };
	windowHeight = { mShader->getUniformLocation("windowHeight"), UniformType::FLOAT };

	mShader->setBinding(image.location, image.bindingSlot);
}


void nex::GaussianBlurVerticalShader::setTexture(const Texture* tex)
{
	mShader->setTexture(tex, &mSampler, image.bindingSlot);
}

void nex::GaussianBlurVerticalShader::setImageWidth(float width)
{
	mShader->setFloat(windowWidth.location, width);
}

void nex::GaussianBlurVerticalShader::setImageHeight(float height)
{
	mShader->setFloat(windowHeight.location, height);
}