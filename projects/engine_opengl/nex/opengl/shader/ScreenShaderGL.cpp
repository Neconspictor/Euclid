#include <nex/opengl/shader/ScreenShaderGL.hpp>
#include <glm/glm.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>
#include <glm/gtc/type_ptr.inl>

using namespace std;
using namespace glm;

ScreenShaderGL::ScreenShaderGL()
{
	mProgram = new ShaderProgramGL("screen_vs.glsl", "screen_fs.glsl");

	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mTexture = { mProgram->getUniformLocation("screenTexture"), UniformType::TEXTURE2D, 0};
}

void ScreenShaderGL::useTexture(const TextureGL* texture)
{
	mProgram->setTexture(mTexture.location, texture, mTexture.textureUnit);
}

void ScreenShaderGL::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}
