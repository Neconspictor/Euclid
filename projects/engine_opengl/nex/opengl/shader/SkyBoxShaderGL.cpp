#include <nex/opengl/shader/SkyBoxShaderGL.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>

using namespace glm;

SkyBoxShaderGL::SkyBoxShaderGL()
{
	mProgram = new ShaderProgramGL("skybox_vs.glsl", "skybox_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mSkyTexture = { mProgram->getUniformLocation("skybox"), UniformType::CUBE_MAP, 0 };
}

void SkyBoxShaderGL::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void SkyBoxShaderGL::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void SkyBoxShaderGL::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void SkyBoxShaderGL::setSkyTexture(const CubeMapGL* texture)
{
	mProgram->setTexture(mSkyTexture.location, texture, mSkyTexture.textureUnit);
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


PanoramaSkyBoxShaderGL::PanoramaSkyBoxShaderGL()
{
	mProgram = new ShaderProgramGL("panorama_skybox_vs.glsl", "panorama_skybox_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mSkyTexture = { mProgram->getUniformLocation("panorama"), UniformType::TEXTURE2D, 0 };
}

void PanoramaSkyBoxShaderGL::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PanoramaSkyBoxShaderGL::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void PanoramaSkyBoxShaderGL::setSkyTexture(const TextureGL* texture)
{
	mProgram->setTexture(mSkyTexture.location, texture, mSkyTexture.textureUnit);
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


EquirectangularSkyBoxShaderGL::EquirectangularSkyBoxShaderGL()
{
	mProgram = new ShaderProgramGL("skybox_equirectangular_vs.glsl", "skybox_equirectangular_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mSkyTexture = { mProgram->getUniformLocation("equirectangularMap"), UniformType::TEXTURE2D, 0 };
}

void EquirectangularSkyBoxShaderGL::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void EquirectangularSkyBoxShaderGL::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void EquirectangularSkyBoxShaderGL::setSkyTexture(const TextureGL * texture)
{
	mProgram->setTexture(mSkyTexture.location, texture, mSkyTexture.textureUnit);
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