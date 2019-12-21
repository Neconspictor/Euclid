#include <nex/effects/ViewSpaceZSpriteShader.hpp>

nex::ViewSpaceZSpriteShader::ViewSpaceZSpriteShader() : SpriteShader(ShaderProgram::create(
	"sprite_vs.glsl", "viewspace_z_sprite_fs.glsl")),
	mDistanceRange(0.0f)
{
	mDistanceRangeUniform = { mProgram->getUniformLocation("cameraRangeDistance"), UniformType::FLOAT };
}

void nex::ViewSpaceZSpriteShader::setCameraDistanceRange(float distanceRange)
{
	mDistanceRange = distanceRange;
}

void nex::ViewSpaceZSpriteShader::update(const Texture* texture, const glm::mat4& mat)
{
	SpriteShader::update(texture, mat);
	mProgram->setFloat(mDistanceRangeUniform.location, mDistanceRange);
}
