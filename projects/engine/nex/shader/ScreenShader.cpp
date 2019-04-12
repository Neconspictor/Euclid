#include <nex/shader/ScreenShader.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>

using namespace nex;

using namespace std;
using namespace glm;

ScreenShader::ScreenShader()
{
	mShader = Shader::create("screen_vs.glsl", "screen_fs.glsl");

	mTransform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };
	mTexture = mShader->createTextureUniform("screenTexture", UniformType::TEXTURE2D, 0);

	mSampler.setMinFilter(TextureFilter::NearestNeighbor);
	mSampler.setMagFilter(TextureFilter::NearestNeighbor);
}

void ScreenShader::useTexture(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mTexture.bindingSlot);
}

void ScreenShader::setMVP(const glm::mat4& mat)
{
	mShader->setMat4(mTransform.location, mat);
}

void ScreenShader::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection)*(*data.view)*(*data.model));
}