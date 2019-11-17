#include <nex/effects/SimpleColorPass.hpp>
#include <nex/material/Material.hpp>

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

void nex::SimpleColorPass::upload(const Material& m)
{
	if (typeid(m)!= typeid(SimpleColorMaterial))
		return;

	const auto& material = (const SimpleColorMaterial&)m;
	setColor(material.getColor());
}