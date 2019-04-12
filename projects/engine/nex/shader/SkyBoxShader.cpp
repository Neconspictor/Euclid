#include <nex/shader/SkyBoxShader.hpp>
#include <nex/RenderBackend.hpp>

using namespace glm;
using namespace nex;

SkyBoxShader::SkyBoxShader()
{
	mShader = Shader::create("skybox_vs.glsl", "skybox_fs.glsl");

	mProjection = { mShader->getUniformLocation("projection"), UniformType::MAT4 };
	mTransform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };
	mView = { mShader->getUniformLocation("view"), UniformType::MAT4 };

	mSkyTexture = mShader->createTextureUniform("skybox", UniformType::CUBE_MAP, 0);
}

void SkyBoxShader::setMVP(const glm::mat4& mat)
{
	mShader->setMat4(mTransform.location, mat);
}

void SkyBoxShader::setProjection(const glm::mat4& mat)
{
	mShader->setMat4(mProjection.location, mat);
}

void SkyBoxShader::setView(const glm::mat4& mat)
{
	mShader->setMat4(mView.location, mat);
}

void SkyBoxShader::setSkyTexture(const CubeMap* texture)
{
	mShader->setTexture(texture, &mSampler, mSkyTexture.bindingSlot);
}

void SkyBoxShader::setupRenderState()
{
	static auto* depthConfig = RenderBackend::get()->getDepthBuffer();
	depthConfig->enableDepthBufferWriting(false);
	depthConfig->setDefaultDepthFunc(CompareFunction::LESS_EQUAL);
}

void SkyBoxShader::reverseRenderState()
{
	static auto* depthConfig = RenderBackend::get()->getDepthBuffer();
	depthConfig->setDefaultDepthFunc(CompareFunction::LESS_EQUAL);
	depthConfig->enableDepthBufferWriting(true);
}


PanoramaSkyBoxShader::PanoramaSkyBoxShader()
{
	mShader = Shader::create("panorama_skybox_vs.glsl", "panorama_skybox_fs.glsl");

	mProjection = { mShader->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mShader->getUniformLocation("view"), UniformType::MAT4 };

	mSkyTexture = mShader->createTextureUniform("panorama", UniformType::TEXTURE2D, 0);
}

void PanoramaSkyBoxShader::setProjection(const glm::mat4& mat)
{
	mShader->setMat4(mProjection.location, mat);
}

void PanoramaSkyBoxShader::setView(const glm::mat4& mat)
{
	mShader->setMat4(mView.location, mat);
}

void PanoramaSkyBoxShader::setSkyTexture(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mSkyTexture.bindingSlot);
}


EquirectangularSkyBoxShader::EquirectangularSkyBoxShader()
{
	mShader = Shader::create("skybox_equirectangular_vs.glsl", "skybox_equirectangular_fs.glsl");

	mProjection = { mShader->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mShader->getUniformLocation("view"), UniformType::MAT4 };
	mSkyTexture = mShader->createTextureUniform("equirectangularMap", UniformType::TEXTURE2D, 0);
}

void EquirectangularSkyBoxShader::setProjection(const glm::mat4& mat)
{
	mShader->setMat4(mProjection.location, mat);
}

void EquirectangularSkyBoxShader::setView(const glm::mat4& mat)
{
	mShader->setMat4(mView.location, mat);
}

void EquirectangularSkyBoxShader::setSkyTexture(const Texture * texture)
{
	mShader->setTexture(texture, &mSampler, mSkyTexture.bindingSlot);
}