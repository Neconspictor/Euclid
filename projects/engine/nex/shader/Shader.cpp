#include <nex/shader/Shader.hpp>
#include <nex/util/StringUtils.hpp>
#include "nex/shader_generator/ShaderSourceFileGenerator.hpp"


nex::UniformTex nex::ShaderProgram::createTextureUniform(const char* name, UniformType type, unsigned bindingSlot)
{
	UniformLocation loc = getUniformLocation(name);
	setBinding(loc, bindingSlot);
	return {loc, type, bindingSlot};
}

std::unique_ptr<nex::ShaderProgram> nex::ShaderProgram::createComputeShader(const FilePath& computeFile, const std::vector<std::string>& defines)
{
	std::vector<UnresolvedShaderStageDesc> unresolved;
	unresolved.resize(1);

	unresolved[0].filePath = computeFile;
	unresolved[0].type = ShaderStageType::COMPUTE;
	unresolved[0].defines = defines;

	ShaderSourceFileGenerator* generator = ShaderSourceFileGenerator::get();
	ProgramSources programSources = generator->generate(unresolved);

	std::vector<Guard<ShaderStage>> shaderStages;
	shaderStages.resize(programSources.descs.size());
	for (unsigned i = 0; i < shaderStages.size(); ++i)
	{
		shaderStages[i] = ShaderStage::compileShaderStage(programSources.descs[i]);
	}

	return create(shaderStages);
}

nex::Shader::Shader(std::unique_ptr<ShaderProgram> program) : mProgram(std::move(program))
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

void nex::Shader::setProgram(std::unique_ptr<ShaderProgram> program)
{
	mProgram = std::move(program);
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

nex::TransformShader::TransformShader(std::unique_ptr<ShaderProgram> program) : Shader(std::move(program))
{
}

nex::ComputeShader::ComputeShader(std::unique_ptr<ShaderProgram> program) : Shader(std::move(program))
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
