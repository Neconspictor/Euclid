#include <nex/shader/post_processing/blur/GaussianBlurPass.hpp>
#include "nex/texture/TextureManager.hpp"

using namespace glm;
using namespace nex;

//height(1024), width(800)

GaussianBlurHorizontalShader::GaussianBlurHorizontalShader()
{
	mShader = Shader::create(
		"post_processing/blur/gaussian_blur_vs.glsl", "post_processing/blur/gaussian_blur_horizontal_fs.glsl");

	image			= { mShader->getUniformLocation("image"), UniformType::TEXTURE2D, 0};
	transform		= { mShader->getUniformLocation("transform"), UniformType::MAT4 };
	windowWidth		= { mShader->getUniformLocation("windowWidth"), UniformType::FLOAT };
	windowHeight	= { mShader->getUniformLocation("windowHeight"), UniformType::FLOAT };

	mShader->setBinding(image.location, image.bindingSlot);
}


void GaussianBlurHorizontalShader::setTexture(const Texture* tex)
{
	mShader->setTexture(tex, &mSampler, image.bindingSlot);
}

void GaussianBlurHorizontalShader::setMVP(const mat4& mvp)
{
	mShader->setMat4(transform.location, mvp);
}

void GaussianBlurHorizontalShader::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection)*(*data.view)*(*data.model));
}

void GaussianBlurHorizontalShader::setImageWidth(float width)
{
	mShader->setFloat(windowWidth.location, width);
}

void GaussianBlurHorizontalShader::setImageHeight(float height)
{
	mShader->setFloat(windowHeight.location, height);
}


//height(1024), width(1024)

GaussianBlurVerticalShader::GaussianBlurVerticalShader()
{
	mShader = Shader::create(
		"post_processing/blur/gaussian_blur_vs.glsl", "post_processing/blur/gaussian_blur_vertical_fs.glsl");

	image = { mShader->getUniformLocation("image"), UniformType::TEXTURE2D, 0 };
	transform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };
	windowWidth = { mShader->getUniformLocation("windowWidth"), UniformType::FLOAT };
	windowHeight = { mShader->getUniformLocation("windowHeight"), UniformType::FLOAT };

	mShader->setBinding(image.location, image.bindingSlot);
}


void GaussianBlurVerticalShader::setTexture(const Texture* tex)
{
	mShader->setTexture(tex, &mSampler, image.bindingSlot);
}

void GaussianBlurVerticalShader::setMVP(const mat4& mvp)
{
	mShader->setMat4(transform.location, mvp);
}

void GaussianBlurVerticalShader::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection)*(*data.view)*(*data.model));
}

void GaussianBlurVerticalShader::setImageWidth(float width)
{
	mShader->setFloat(windowWidth.location, width);
}

void GaussianBlurVerticalShader::setImageHeight(float height)
{
	mShader->setFloat(windowHeight.location, height);
}