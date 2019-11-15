#include <nex/effects/SkyBoxPass.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/material/Material.hpp>

using namespace glm;
using namespace nex;

SkyBoxPass::SkyBoxPass()
{
	mProgram = ShaderProgram::create("skybox_vs.glsl", "skybox_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };

	mSkyTexture = mProgram->createTextureUniform("skybox", UniformType::CUBE_MAP, 0);
}

void SkyBoxPass::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void SkyBoxPass::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void SkyBoxPass::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void SkyBoxPass::setSkyTexture(const CubeMap* texture)
{
	mProgram->setTexture(texture, Sampler::getLinear(), mSkyTexture.bindingSlot);
}


PanoramaSkyBoxPass::PanoramaSkyBoxPass()
{
	mProgram = ShaderProgram::create("panorama_skybox_vs.glsl", "panorama_skybox_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };

	mSkyTexture = mProgram->createTextureUniform("panorama", UniformType::TEXTURE2D, 0);
}

void PanoramaSkyBoxPass::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PanoramaSkyBoxPass::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void PanoramaSkyBoxPass::setSkyTexture(const Texture* texture)
{
	mProgram->setTexture(texture, Sampler::getLinear(), mSkyTexture.bindingSlot);
}


EquirectangularSkyBoxPass::EquirectangularSkyBoxPass()
{
	mProgram = ShaderProgram::create("skybox_equirectangular_vs.glsl", "skybox_equirectangular_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mSkyTexture = mProgram->createTextureUniform("equirectangularMap", UniformType::TEXTURE2D, 0);
}

void EquirectangularSkyBoxPass::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void EquirectangularSkyBoxPass::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void EquirectangularSkyBoxPass::setSkyTexture(const Texture * texture)
{
	mProgram->setTexture(texture, Sampler::getLinear(), mSkyTexture.bindingSlot);
}