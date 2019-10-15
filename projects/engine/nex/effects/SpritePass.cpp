#include <nex/effects/SpritePass.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>

using namespace nex;

using namespace std;
using namespace glm;

SpritePass::SpritePass() : SpritePass(Shader::create("sprite_vs.glsl", "sprite_fs.glsl"))
{
}

nex::SpritePass::SpritePass(std::unique_ptr<Shader> shader) : Pass(std::move(shader))
{
	mTexture = mShader->createTextureUniform("sprite", UniformType::TEXTURE2D, 0);
	mTransform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };

	mSampler.setMinFilter(TextureFilter::NearestNeighbor);
	mSampler.setMagFilter(TextureFilter::NearestNeighbor);
}

nex::SpritePass::~SpritePass() = default;

void nex::SpritePass::setTransform(const glm::mat4& mat)
{
	mShader->setMat4(mTransform.location, mat);
}

void SpritePass::setTexture(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mTexture.bindingSlot);
}

nex::DepthSpritePass::DepthSpritePass() : SpritePass(Shader::create("sprite_vs.glsl", "depth_sprite_fs.glsl"))
{
}

nex::DepthSpritePass::~DepthSpritePass() = default;
