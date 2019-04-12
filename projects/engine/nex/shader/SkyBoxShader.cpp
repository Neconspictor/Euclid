#include <nex/shader/SkyBoxShader.hpp>
#include <nex/RenderBackend.hpp>

using namespace glm;
using namespace nex;

SkyBoxShader::SkyBoxShader()
{
	mProgram = ShaderProgram::create("skybox_vs.glsl", "skybox_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };

	mSkyTexture = mProgram->createTextureUniform("skybox", UniformType::CUBE_MAP, 0);
}

void SkyBoxShader::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void SkyBoxShader::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void SkyBoxShader::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void SkyBoxShader::setSkyTexture(const CubeMap* texture)
{
	mProgram->setTexture(texture, &mSampler, mSkyTexture.bindingSlot);
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
	mProgram = ShaderProgram::create("panorama_skybox_vs.glsl", "panorama_skybox_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };

	mSkyTexture = mProgram->createTextureUniform("panorama", UniformType::TEXTURE2D, 0);
}

void PanoramaSkyBoxShader::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PanoramaSkyBoxShader::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void PanoramaSkyBoxShader::setSkyTexture(const Texture* texture)
{
	mProgram->setTexture(texture, &mSampler, mSkyTexture.bindingSlot);
}


EquirectangularSkyBoxShader::EquirectangularSkyBoxShader()
{
	mProgram = ShaderProgram::create("skybox_equirectangular_vs.glsl", "skybox_equirectangular_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mSkyTexture = mProgram->createTextureUniform("equirectangularMap", UniformType::TEXTURE2D, 0);
}

void EquirectangularSkyBoxShader::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void EquirectangularSkyBoxShader::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void EquirectangularSkyBoxShader::setSkyTexture(const Texture * texture)
{
	mProgram->setTexture(texture, &mSampler, mSkyTexture.bindingSlot);
}