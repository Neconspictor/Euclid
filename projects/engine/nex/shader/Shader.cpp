#include <nex/shader/Shader.hpp>
#include <nex/util/StringUtils.hpp>

nex::ShaderProgram::ShaderProgram(void* impl): mImpl(impl), mIsBound(false)
{
}

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

nex::ShaderType nex::stringToShaderEnum(const std::string& str)
{
	return nex::util::stringToEnum(str, nex::shaderEnumConversion);
}

std::ostream& nex::operator<<(std::ostream& os, nex::ShaderType shader)
{
	os << nex::util::enumToString(shader, nex::shaderEnumConversion);
	return os;
}

nex::TransformShaderGL::TransformShaderGL(ShaderProgram * program) : Shader(program)
{
}

std::ostream& nex::operator<<(std::ostream& os, nex::ShaderStageType stageType)
{
	static std::string const table[] =
	{
		"COMPUTE",
		"FRAGMENT",
		"GEOMETRY",
		"TESSELATION_CONTROL",
		"TESSELATION_EVALUATION",
		"VERTEX",
	};

	static const unsigned size = static_cast<unsigned>(nex::ShaderStageType::SHADER_STAGE_LAST) + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "NeX error: ShaderStageType descriptor list doesn't match number of supported shader stages");

	os << table[static_cast<unsigned>(stageType)];
	return os;
}