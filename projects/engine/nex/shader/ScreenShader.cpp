#include <nex/shader/ScreenShader.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>

using namespace nex;

using namespace std;
using namespace glm;

ScreenShader::ScreenShader()
{
	mProgram = ShaderProgram::create("screen_vs.glsl", "screen_fs.glsl");

	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mTexture = mProgram->createTextureUniform("screenTexture", UniformType::TEXTURE2D, 0);
}

void ScreenShader::useTexture(const Texture* texture)
{
	mProgram->setTexture(texture, mTexture.bindingSlot);
}

void ScreenShader::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void ScreenShader::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection)*(*data.view)*(*data.model));
}