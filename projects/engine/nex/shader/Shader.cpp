#include <nex/shader/Shader.hpp>
#include "nex/util/StringUtils.hpp"

nex::Shader::Shader(ShaderProgram* program) : mProgram(program)
{
}

void nex::Shader::bind()
{
	mProgram->bind();
}

nex::ShaderProgram* nex::Shader::getProgram()
{
	return mProgram.get();
}

void nex::Shader::setProgram(ShaderProgram* program)
{
	mProgram = program;
}

void nex::Shader::unbind()
{
	mProgram->unbind();
}

void nex::Shader::onModelMatrixUpdate(const glm::mat4& modelMatrix)
{
}

void nex::Shader::onMaterialUpdate(const Material* material)
{
}

void nex::Shader::setupRenderState()
{
}

void nex::Shader::reverseRenderState()
{
}

nex::ShaderType stringToShaderEnum(const std::string& str)
{
	return nex::util::stringToEnum(str, nex::shaderEnumConversion);
}

std::ostream& operator<<(std::ostream& os, nex::ShaderType shader)
{
	os << nex::util::enumToString(shader, nex::shaderEnumConversion);
	return os;
}

nex::TransformShaderGL::TransformShaderGL(ShaderProgram * program) : Shader(program)
{
}