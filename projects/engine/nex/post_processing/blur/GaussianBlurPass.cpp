#include <nex/post_processing/blur/GaussianBlurPass.hpp>
#include "nex/texture/TextureManager.hpp"


nex::GaussianBlurHorizontalShader::GaussianBlurHorizontalShader()
{
	mProgram = ShaderProgram::create(
		"post_processing/blur/gaussian_blur_vs.glsl", "post_processing/blur/gaussian_blur_horizontal_fs.glsl");

	image			= { mProgram->getUniformLocation("image"), UniformType::TEXTURE2D, 0};
	windowWidth		= { mProgram->getUniformLocation("windowWidth"), UniformType::FLOAT };
	windowHeight	= { mProgram->getUniformLocation("windowHeight"), UniformType::FLOAT };

	mProgram->setBinding(image.location, image.bindingSlot);
}


void nex::GaussianBlurHorizontalShader::setTexture(const Texture* tex)
{
	mProgram->setTexture(tex, Sampler::getLinear(), image.bindingSlot);
}

void nex::GaussianBlurHorizontalShader::setImageWidth(float width)
{
	mProgram->setFloat(windowWidth.location, width);
}

void nex::GaussianBlurHorizontalShader::setImageHeight(float height)
{
	mProgram->setFloat(windowHeight.location, height);
}


//height(1024), width(1024)

nex::GaussianBlurVerticalShader::GaussianBlurVerticalShader()
{
	mProgram = ShaderProgram::create(
		"post_processing/blur/gaussian_blur_vs.glsl", "post_processing/blur/gaussian_blur_vertical_fs.glsl");

	image = { mProgram->getUniformLocation("image"), UniformType::TEXTURE2D, 0 };
	windowWidth = { mProgram->getUniformLocation("windowWidth"), UniformType::FLOAT };
	windowHeight = { mProgram->getUniformLocation("windowHeight"), UniformType::FLOAT };

	mProgram->setBinding(image.location, image.bindingSlot);
}


void nex::GaussianBlurVerticalShader::setTexture(const Texture* tex)
{
	mProgram->setTexture(tex, Sampler::getLinear(), image.bindingSlot);
}

void nex::GaussianBlurVerticalShader::setImageWidth(float width)
{
	mProgram->setFloat(windowWidth.location, width);
}

void nex::GaussianBlurVerticalShader::setImageHeight(float height)
{
	mProgram->setFloat(windowHeight.location, height);
}