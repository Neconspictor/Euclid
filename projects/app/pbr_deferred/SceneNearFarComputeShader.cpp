#include "SceneNearFarComputeShader.hpp"
#include <nex/shader/Shader.hpp>
#include "nex/shader_generator/ShaderSourceFileGenerator.hpp"
#include "nex/RenderBackend.hpp"
#include <glm/glm.hpp>
#include "nex/shader/ShaderBuffer.hpp"


void nex::SceneNearFarComputeShader::setConstants(float positiveViewNearZ, float positiveViewFarZ, const glm::mat4& projection)
{
	mConstantBuffer->bind();
	Constant data;
	data.mCameraNearFar = glm::vec4(positiveViewNearZ, positiveViewFarZ, 0.0, 0.0);
	data.mCameraProj = projection;

	mConstantBuffer->update(&data, sizeof(Constant));
}

nex::SceneNearFarComputeShader::SceneNearFarComputeShader() : ComputeShader()
{
	std::vector<UnresolvedShaderStageDesc> unresolved;
	unresolved.resize(1);

	unresolved[0].filePath = "test/compute/scene_bounds_cs.glsl";
	unresolved[0].type = ShaderStageType::COMPUTE;

	ShaderSourceFileGenerator* generator = ShaderSourceFileGenerator::get();
	ProgramSources programSources = generator->generate(unresolved);

	std::vector<Guard<ShaderStage>> shaderStages;
	shaderStages.resize(programSources.descs.size());
	for (unsigned i = 0; i < shaderStages.size(); ++i)
	{
		auto path = generator->getFileSystem()->resolvePath("test/compute");
		path += "/scene_bounds_cs-resolved.glsl";
		FileSystem::writeToFile(path.generic_string(), programSources.descs[i].root.resolvedSource);
		shaderStages[i] = ShaderStage::compileShaderStage(programSources.descs[i]);
	}

	mProgram = ShaderProgram::create(shaderStages);

	mConstantBuffer = std::make_unique<ShaderStorageBuffer>(0, sizeof(Constant), ShaderBuffer::UsageHint::DYNAMIC_DRAW);
	mWriteOutBuffer = std::make_unique<ShaderStorageBuffer>(1, sizeof(WriteOut), ShaderBuffer::UsageHint::DYNAMIC_DRAW);


	bind();
	reset();

	Constant constant;
	mConstantBuffer->bind();
	mConstantBuffer->update(&constant, sizeof(constant));

	mDepthTextureUniform = {mProgram->getUniformLocation("depthTexture"), UniformType::TEXTURE2D};
}

nex::SceneNearFarComputeShader::WriteOut nex::SceneNearFarComputeShader::readResult()
{
	mWriteOutBuffer->bind();
	WriteOut* result = (WriteOut*)mWriteOutBuffer->map(ShaderBuffer::Access::READ_WRITE);
	WriteOut copy(*result);
	mWriteOutBuffer->unmap();
	return copy;
}

void nex::SceneNearFarComputeShader::setDepthTexture(Texture* depth)
{
	mProgram->setTexture(mDepthTextureUniform.location, depth, 0);
}

nex::ShaderStorageBuffer* nex::SceneNearFarComputeShader::getConstantBuffer()
{
	return mConstantBuffer.get();
}

nex::ShaderStorageBuffer* nex::SceneNearFarComputeShader::getWriteOutBuffer()
{
	return mWriteOutBuffer.get();
}

void nex::SceneNearFarComputeShader::reset()
{
	WriteOut out;
	out.minMax.x = FLT_MAX;
	out.minMax.y = 0;

	mWriteOutBuffer->bind();
	mWriteOutBuffer->update(&out, sizeof(WriteOut));
}