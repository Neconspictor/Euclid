#include <nex/effects/SimpleColorPass.hpp>

nex::SimpleColorPass::SimpleColorPass() : TransformShader(ShaderProgram::create("simpleColor_vs.glsl", "simpleColor_fs.glsl")), mColor(1.0f)
{
	bind();
	mColorUniform = {mProgram->getUniformLocation("objectColor"), UniformType::VEC4};
	mProgram->setVec4(mColorUniform.location, mColor);
}

void nex::SimpleColorPass::setColor(const glm::vec4 color)
{
	mColor = color;
	mProgram->setVec4(mColorUniform.location, mColor);
}