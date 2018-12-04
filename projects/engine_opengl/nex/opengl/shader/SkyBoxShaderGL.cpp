#include <nex/opengl/shader/SkyBoxShaderGL.hpp>

using namespace glm;
using namespace nex;

SkyBoxShader::SkyBoxShader()
{
	mProgram = ShaderProgram::create("skybox_vs.glsl", "skybox_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mSkyTexture = { mProgram->getUniformLocation("skybox"), UniformType::CUBE_MAP, 0 };
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
	mProgram->setTexture(mSkyTexture.location, texture, mSkyTexture.bindingSlot);
}

void SkyBoxShader::setupRenderState()
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
}

void SkyBoxShader::reverseRenderState()
{
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glDepthMask(GL_TRUE);
}

//TODO
/*
void SkyBoxShaderGL::afterDrawing(const MeshGL& mesh)
{
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glDepthMask(GL_TRUE);
}

void SkyBoxShaderGL::beforeDrawing(const MeshGL& mesh)
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
}*/


PanoramaSkyBoxShader::PanoramaSkyBoxShader()
{
	mProgram = ShaderProgram::create("panorama_skybox_vs.glsl", "panorama_skybox_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mSkyTexture = { mProgram->getUniformLocation("panorama"), UniformType::TEXTURE2D, 0 };
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
	mProgram->setTexture(mSkyTexture.location, texture, mSkyTexture.bindingSlot);
}

//TODO
/*
void PanoramaSkyBoxShaderGL::afterDrawing(const MeshGL& mesh)
{
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glDepthMask(GL_TRUE);
}

void PanoramaSkyBoxShaderGL::beforeDrawing(const MeshGL& mesh)
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
}*/


EquirectangularSkyBoxShader::EquirectangularSkyBoxShader()
{
	mProgram = ShaderProgram::create("skybox_equirectangular_vs.glsl", "skybox_equirectangular_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mSkyTexture = { mProgram->getUniformLocation("equirectangularMap"), UniformType::TEXTURE2D, 0 };
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
	mProgram->setTexture(mSkyTexture.location, texture, mSkyTexture.bindingSlot);
}

//TODO
/*
void EquirectangularSkyBoxShaderGL::afterDrawing(const MeshGL& mesh)
{
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
	glDepthMask(GL_TRUE);
}

void EquirectangularSkyBoxShaderGL::beforeDrawing(const MeshGL& mesh)
{
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
}*/