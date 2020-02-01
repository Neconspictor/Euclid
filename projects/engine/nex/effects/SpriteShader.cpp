#include <nex/effects/SpriteShader.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>

using namespace nex;

using namespace std;
using namespace glm;

SpriteShader::SpriteShader() : SpriteShader(ShaderProgram::create("sprite_vs.glsl", "sprite_fs.glsl"))
{
}

nex::SpriteShader::SpriteShader(std::unique_ptr<ShaderProgram> shader) : Shader(std::move(shader))
{
	mTexture = mProgram->createTextureUniform("sprite", UniformType::TEXTURE2D, 0);
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
}

nex::SpriteShader::~SpriteShader() = default;

void nex::SpriteShader::update(const Texture * texture, const glm::mat4 & mat)
{
	mProgram->setMat4(mTransform.location, mat);
	mProgram->setTexture(texture, Sampler::getLinear(), mTexture.bindingSlot);
}

nex::DepthSpriteShader::DepthSpriteShader() : SpriteShader(ShaderProgram::create("sprite_vs.glsl", "depth_sprite_fs.glsl"))
{
}

nex::DepthSpriteShader::~DepthSpriteShader() = default;
