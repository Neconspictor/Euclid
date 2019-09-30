#include <nex/shader/SpritePass.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>

using namespace nex;

using namespace std;
using namespace glm;

SpritePass::SpritePass()
{
	mShader = Shader::create("sprite_vs.glsl", "sprite_fs.glsl");

	mTexture = mShader->createTextureUniform("sprite", UniformType::TEXTURE2D, 0);
	mTransform = {mShader->getUniformLocation("transform"), UniformType::MAT4};

	mSampler.setMinFilter(TextureFilter::NearestNeighbor);
	mSampler.setMagFilter(TextureFilter::NearestNeighbor);
}

void nex::SpritePass::setTransform(const glm::mat4& mat)
{
	mShader->setMat4(mTransform.location, mat);
}

void SpritePass::setTexture(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mTexture.bindingSlot);
}