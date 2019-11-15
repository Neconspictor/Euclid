#include <nex/effects/SpritePass.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>

using namespace nex;

using namespace std;
using namespace glm;

SpritePass::SpritePass() : SpritePass(ShaderProgram::create("sprite_vs.glsl", "sprite_fs.glsl"))
{
}

nex::SpritePass::SpritePass(std::unique_ptr<ShaderProgram> shader) : Shader(std::move(shader))
{
	mTexture = mProgram->createTextureUniform("sprite", UniformType::TEXTURE2D, 0);
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
}

nex::SpritePass::~SpritePass() = default;

void nex::SpritePass::setTransform(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void SpritePass::setTexture(const Texture* texture)
{
	mProgram->setTexture(texture, Sampler::getPoint(), mTexture.bindingSlot);
}

nex::DepthSpritePass::DepthSpritePass() : SpritePass(ShaderProgram::create("sprite_vs.glsl", "depth_sprite_fs.glsl"))
{
}

nex::DepthSpritePass::~DepthSpritePass() = default;
