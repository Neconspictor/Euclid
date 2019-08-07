#include <nex/shader/SimpleColorPass.hpp>

nex::SimpleColorPass::SimpleColorPass() : TransformPass(Shader::create("simpleColor_vs.glsl", "simpleColor_fs.glsl")), mColor(1.0f)
{
	bind();
	mColorUniform = {mShader->getUniformLocation("objectColor"), UniformType::VEC4};
	mShader->setVec4(mColorUniform.location, mColor);
}

void nex::SimpleColorPass::setColor(const glm::vec4 color)
{
	mColor = color;
	mShader->setVec4(mColorUniform.location, mColor);
}

nex::SimpleColorTechnique::SimpleColorTechnique() : Technique(&mSimpleColorPass)
{
}

nex::SimpleColorPass* nex::SimpleColorTechnique::getSimpleColorPass()
{
	return &mSimpleColorPass;
}