#include <nex/opengl/shader/post_processing/blur/GaussianBlurShaderGL.hpp>

using namespace glm;

//height(1024), width(800)

GaussianBlurHorizontalShaderGL::GaussianBlurHorizontalShaderGL()
{
	mProgram = new ShaderProgramGL(
		"post_processing/blur/gaussian_blur_vs.glsl", "post_processing/blur/gaussian_blur_horizontal_fs.glsl");

	image			= { mProgram->getUniformLocation("image"), UniformType::TEXTURE2D, 0};
	transform		= { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	windowWidth		= { mProgram->getUniformLocation("windowWidth"), UniformType::FLOAT };
	windowHeight	= { mProgram->getUniformLocation("windowHeight"), UniformType::FLOAT };
}


void GaussianBlurHorizontalShaderGL::setTexture(const TextureGL* tex)
{
	mProgram->setTexture(image.location, tex, image.textureUnit);
}

void GaussianBlurHorizontalShaderGL::setMVP(const mat4& mvp)
{
	mProgram->setMat4(transform.location, mvp);
}

void GaussianBlurHorizontalShaderGL::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection)*(*data.view)*(*data.model));
}

void GaussianBlurHorizontalShaderGL::setImageWidth(float width)
{
	mProgram->setInt(windowWidth.location, width);
}

void GaussianBlurHorizontalShaderGL::setImageHeight(float height)
{
	mProgram->setInt(windowHeight.location, height);
}


//height(1024), width(1024)

GaussianBlurVerticalShaderGL::GaussianBlurVerticalShaderGL()
{
	mProgram = new ShaderProgramGL(
		"post_processing/blur/gaussian_blur_vs.glsl", "post_processing/blur/gaussian_blur_vertical_fs.glsl");

	image = { mProgram->getUniformLocation("image"), UniformType::TEXTURE2D, 0 };
	transform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	windowWidth = { mProgram->getUniformLocation("windowWidth"), UniformType::FLOAT };
	windowHeight = { mProgram->getUniformLocation("windowHeight"), UniformType::FLOAT };
}


void GaussianBlurVerticalShaderGL::setTexture(const TextureGL* tex)
{
	mProgram->setTexture(image.location, tex, image.textureUnit);
}

void GaussianBlurVerticalShaderGL::setMVP(const mat4& mvp)
{
	mProgram->setMat4(transform.location, mvp);
}

void GaussianBlurVerticalShaderGL::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection)*(*data.view)*(*data.model));
}

void GaussianBlurVerticalShaderGL::setImageWidth(float width)
{
	mProgram->setInt(windowWidth.location, width);
}

void GaussianBlurVerticalShaderGL::setImageHeight(float height)
{
	mProgram->setInt(windowHeight.location, height);
}