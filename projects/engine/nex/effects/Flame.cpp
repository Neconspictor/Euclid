#include <nex/effects/Flame.hpp>

nex::FlameShader::FlameShader() : TransformShader(ShaderProgram::create("effects/flame_vs.glsl", "effects/flame_fs.glsl"))
{
	mStructure = mProgram->createTextureUniform("structureTex", UniformType::TEXTURE2D, 0);
	mBaseColor = { mProgram->getUniformLocation("baseColor"), UniformType::VEC4 };
	mTime = { mProgram->getUniformLocation("time"), UniformType::FLOAT };
}

void nex::FlameShader::setStructure(const Texture* structure, const Sampler* sampler)
{
	mProgram->setTexture(structure, sampler, mStructure.bindingSlot);
}

void nex::FlameShader::setBaseColor(const glm::vec4& color)
{
	mProgram->setVec4(mBaseColor.location, color);
}

void nex::FlameShader::setTime(float time)
{
	mProgram->setFloat(mTime.location, time);
}

void nex::FlameShader::updateConstants(const Constants& constants)
{
	TransformShader::updateConstants(constants);
	setTime(constants.time);
}

void nex::FlameShader::updateMaterial(const Material& material)
{
	const FlameMaterial* flameMaterial;
	try {
		flameMaterial = &dynamic_cast<const FlameMaterial&>(material);
	}
	catch (std::bad_cast & e) {
		throw_with_trace(e);
	}


	setStructure(flameMaterial->structure, flameMaterial->structureSampler.get());
	setBaseColor(flameMaterial->baseColor);
}

nex::FlameMaterial::FlameMaterial(FlameShader* shader, 
	const Texture* structure, 
	std::unique_ptr<Sampler> sampler,
	const glm::vec4& baseColor) : 
	Material(shader),
	structure(structure), 
	structureSampler(std::move(sampler)),
	baseColor(baseColor)

{
	mRenderState.doBlend = true;
	mRenderState.blendDesc = BlendDesc::createAlphaTransparency(); // { BlendFunc::SOURCE_ALPHA, BlendFunc::ONE, BlendOperation::ADD }; //BlendDesc::createAlphaTransparency();//
	mRenderState.doShadowCast = false;
	mRenderState.doShadowReceive = false;
	mRenderState.doCullFaces = false;
	mRenderState.doDepthWrite = false;
}

nex::FlameMaterialLoader::FlameMaterialLoader(
	FlameShader* shader,
	const Texture* structure, 
	const SamplerDesc& structureSamplerDesc, 
	const glm::vec4& baseColor) :
	AbstractMaterialLoader(nullptr),
	mStructure(structure),
	mStructureSamplerDesc(structureSamplerDesc),
	mBaseColor(baseColor),
	mShader(shader)
{
}

void nex::FlameMaterialLoader::loadShadingMaterial(const std::filesystem::path& meshPathAbsolute, const aiScene* scene, MaterialStore& store, unsigned materialIndex) const
{
}

std::unique_ptr<nex::Material> nex::FlameMaterialLoader::createMaterial(const MaterialStore& store) const
{
	return std::make_unique<FlameMaterial>(
		mShader,
		mStructure, 
		std::make_unique<Sampler>(mStructureSamplerDesc),
		mBaseColor);
}