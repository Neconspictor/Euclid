#include <nex/shader/SkyBoxPass.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/material/Material.hpp>

using namespace glm;
using namespace nex;

SkyBoxPass::SkyBoxPass()
{
	mShader = Shader::create("skybox_vs.glsl", "skybox_fs.glsl");

	mProjection = { mShader->getUniformLocation("projection"), UniformType::MAT4 };
	mTransform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };
	mView = { mShader->getUniformLocation("view"), UniformType::MAT4 };

	mSkyTexture = mShader->createTextureUniform("skybox", UniformType::CUBE_MAP, 0);
}

void SkyBoxPass::setMVP(const glm::mat4& mat)
{
	mShader->setMat4(mTransform.location, mat);
}

void SkyBoxPass::setProjection(const glm::mat4& mat)
{
	mShader->setMat4(mProjection.location, mat);
}

void SkyBoxPass::setView(const glm::mat4& mat)
{
	mShader->setMat4(mView.location, mat);
}

void SkyBoxPass::setSkyTexture(const CubeMap* texture)
{
	mShader->setTexture(texture, &mSampler, mSkyTexture.bindingSlot);
}


PanoramaSkyBoxPass::PanoramaSkyBoxPass()
{
	mShader = Shader::create("panorama_skybox_vs.glsl", "panorama_skybox_fs.glsl");

	mProjection = { mShader->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mShader->getUniformLocation("view"), UniformType::MAT4 };

	mSkyTexture = mShader->createTextureUniform("panorama", UniformType::TEXTURE2D, 0);
}

void PanoramaSkyBoxPass::setProjection(const glm::mat4& mat)
{
	mShader->setMat4(mProjection.location, mat);
}

void PanoramaSkyBoxPass::setView(const glm::mat4& mat)
{
	mShader->setMat4(mView.location, mat);
}

void PanoramaSkyBoxPass::setSkyTexture(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mSkyTexture.bindingSlot);
}


EquirectangularSkyBoxPass::EquirectangularSkyBoxPass()
{
	mShader = Shader::create("skybox_equirectangular_vs.glsl", "skybox_equirectangular_fs.glsl");

	mProjection = { mShader->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mShader->getUniformLocation("view"), UniformType::MAT4 };
	mSkyTexture = mShader->createTextureUniform("equirectangularMap", UniformType::TEXTURE2D, 0);
}

void EquirectangularSkyBoxPass::setProjection(const glm::mat4& mat)
{
	mShader->setMat4(mProjection.location, mat);
}

void EquirectangularSkyBoxPass::setView(const glm::mat4& mat)
{
	mShader->setMat4(mView.location, mat);
}

void EquirectangularSkyBoxPass::setSkyTexture(const Texture * texture)
{
	mShader->setTexture(texture, &mSampler, mSkyTexture.bindingSlot);
}