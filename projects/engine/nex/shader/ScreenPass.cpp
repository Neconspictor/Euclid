#include <nex/shader/ScreenPass.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>

using namespace nex;

using namespace std;
using namespace glm;

ScreenPass::ScreenPass()
{
	mShader = Shader::create("screen_vs.glsl", "screen_fs.glsl");

	mTexture = mShader->createTextureUniform("screenTexture", UniformType::TEXTURE2D, 0);

	mSampler.setMinFilter(TextureFilter::NearestNeighbor);
	mSampler.setMagFilter(TextureFilter::NearestNeighbor);
}

void ScreenPass::useTexture(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mTexture.bindingSlot);
}