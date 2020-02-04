#include <nex/pbr/PbrPass.hpp>
#include <glm/glm.hpp>
#include <nex/mesh/Mesh.hpp>
#include <nex/material/Material.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nex/GI/Probe.hpp>
#include <nex/GI/GlobalIllumination.hpp>
#include <nex/cluster/Cluster.hpp>

using namespace glm;
using namespace std;
using namespace nex;

PbrGeometryData::PbrGeometryData(ShaderProgram* shader) : PbrBaseCommon(shader)
{
	assert(mShader != nullptr);

	// mesh material
	mAlbedoMap = mShader->createTextureUniform("material.albedoMap", UniformType::TEXTURE2D, ALBEDO_BINDING_POINT);
	mAmbientOcclusionMap = mShader->createTextureUniform("material.aoMap", UniformType::TEXTURE2D, AO_BINDING_POINT);
	mMetalMap = mShader->createTextureUniform("material.metallicMap", UniformType::TEXTURE2D, METALLIC_BINDING_POINT);
	mNormalMap = mShader->createTextureUniform("material.normalMap", UniformType::TEXTURE2D, NORMAL_BINDING_POINT);
	mRoughnessMap = mShader->createTextureUniform("material.roughnessMap", UniformType::TEXTURE2D, ROUGHNESS_BINDING_POINT);

	mNearFarPlane = { mShader->getUniformLocation("nearFarPlane"), UniformType::VEC2 };

	mDiffuseReflectionProbeID = { mShader->getUniformLocation("material.diffuseReflectionProbeID"), UniformType::UINT };
	mSpecularReflectionProbeID = { mShader->getUniformLocation("material.specularReflectionProbeID"), UniformType::UINT };
}

void nex::PbrGeometryData::setAlbedoMap(const Texture* albedo)
{
	mShader->setTexture(albedo, Sampler::getDefaultImage(), mAlbedoMap.bindingSlot);
}

void nex::PbrGeometryData::setAoMap(const Texture* ao)
{
	mShader->setTexture(ao, Sampler::getDefaultImage(), mAmbientOcclusionMap.bindingSlot);
}

void nex::PbrGeometryData::setMetalMap(const Texture* metal)
{
	mShader->setTexture(metal, Sampler::getDefaultImage(), mMetalMap.bindingSlot);
}

void nex::PbrGeometryData::setNormalMap(const Texture* normal)
{
	mShader->setTexture(normal, Sampler::getDefaultImage(), mNormalMap.bindingSlot);
}

void nex::PbrGeometryData::setRoughnessMap(const Texture* roughness)
{
	mShader->setTexture(roughness, Sampler::getDefaultImage(), mRoughnessMap.bindingSlot);
}

void nex::PbrGeometryData::setData(const PbrMaterial::Data& data)
{
	mShader->setUInt(mDiffuseReflectionProbeID.location, data.diffuseReflectionProbeID);
	mShader->setUInt(mSpecularReflectionProbeID.location, data.specularReflectionProbeID);
}

void PbrGeometryData::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mShader->setVec2(mNearFarPlane.location, nearFarPlane);
}

PbrBaseCommon::PbrBaseCommon(ShaderProgram* shader) : mShader(shader)
{
}

void PbrBaseCommon::setProgram(ShaderProgram* shader)
{
	mShader = shader;
}

void PbrGeometryData::updateConstants(const RenderContext& constants)
{
	setNearFarPlane(constants.camera->getNearFarPlaneViewSpace());
}

void nex::PbrLightingData::setIrradianceArrayIndex(float index)
{
	mShader->setFloat(mIrradianceArrayIndex.location, index);
}

void nex::PbrLightingData::setReflectionArrayIndex(float index)
{
	mShader->setFloat(mReflectionArrayIndex.location, index);
}

void PbrLightingData::setBrdfLookupTexture(const Texture* brdfLUT)
{
	//mProgram->setTextureByHandle(mBrdfLUT.location, brdfLUT);
	mShader->setTexture(brdfLUT, &mSampler, mBrdfLUT.bindingSlot);
}

void nex::PbrLightingData::setIrradianceMaps(const Texture1DArray * texture)
{
	mShader->setTexture(texture, Sampler::getPoint(), mIrradianceMaps.bindingSlot);
}

void nex::PbrLightingData::setReflectionMaps(const CubeMapArray * texture)
{
	mShader->setTexture(texture, &mReflectionSampler, mReflectionMaps.bindingSlot);
}

void PbrLightingData::setCascadedDepthMap(const Texture* cascadedDepthMap)
{
	mShader->setTexture(cascadedDepthMap, &mCascadedShadowMapSampler, mCascadedDepthMap.bindingSlot);
	//mProgram->setTextureByHandle(mCascadedDepthMap.location, cascadedDepthMap);
}

//void PbrCommonLightingPass::setCascadedData(const CascadedShadow::CascadeData& cascadedData)
//{
//	auto* buffer = (ShaderStorageBuffer*)&cascadeBufferUBO; // UniformBuffer ShaderStorageBuffer
//	buffer->bind();
//
//	assert(cascadeBufferUBO.getSize() == cascadedData.shaderBuffer.size());
//
//	buffer->update(cascadedData.shaderBuffer.data(), cascadedData.shaderBuffer.size(), 0);
//}

void nex::PbrLightingData::setLightDirectionWS(const glm::vec4& direction)
{
	mShader->setVec4(mLightDirectionWS.location, direction);
}

void PbrLightingData::setEyeLightDirection(const glm::vec4& direction)
{
	mShader->setVec4(mEyeLightDirection.location, direction);
}

void PbrLightingData::setLightColor(const glm::vec4& color)
{
	mShader->setVec4(mLightColor.location, color);
}

void PbrLightingData::setLightPower(float power)
{
	mShader->setFloat(mLightPower.location, power);
}

void PbrLightingData::setAmbientLightPower(float power)
{
	mShader->setFloat(mAmbientLightPower.location, power);
}

void PbrLightingData::setShadowStrength(float strength)
{
	mShader->setFloat(mShadowStrength.location, strength);
}

void PbrLightingData::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mShader->setVec2(mNearFarPlane.location, nearFarPlane);
}

nex::PbrLightingData::PbrLightingData(ShaderProgram * shader, GlobalIllumination* globalIllumination, 
	CascadedShadow* cascadedShadow, unsigned envLightBindingPoint,
	unsigned envLightGlobalLightIndicesBindingPoint,
	unsigned envLightLightGridsBindingPoint,
	unsigned clustersAABBBindingPoint) :
	PbrBaseCommon(shader),
	mEnvLightBindingPoint(envLightBindingPoint),
	mGlobalIllumination(globalIllumination),
	
	//cascadeBufferUBO(
	//	CSM_CASCADE_BUFFER_BINDING_POINT, 
	//	CascadedShadow::CascadeData::calcCascadeDataByteSize(cascadedShadow->getCascadeData().numCascades),
	//	ShaderBuffer::UsageHint::DYNAMIC_COPY), mProbe(nullptr),
	mCascadeShadow(cascadedShadow),
	mEnvLightGlobalLightIndicesBindingPoint(envLightGlobalLightIndicesBindingPoint),
	mEnvLightLightGridsBindingPoint(envLightLightGridsBindingPoint),
	mClustersAABBBindingPoint(clustersAABBBindingPoint)
{
	assert(mShader != nullptr);

	mCascadedShadowMapSampler.setMinFilter(TexFilter::Nearest);
	mCascadedShadowMapSampler.setMagFilter(TexFilter::Nearest);

	// ibl
	//mIrradianceMap = mProgram->createTextureHandleUniform("irradianceMap", UniformType::CUBE_MAP);
	//mPrefilterMap = mProgram->createTextureHandleUniform("prefilterMap", UniformType::CUBE_MAP);
	//mBrdfLUT = mProgram->createTextureHandleUniform("brdfLUT", UniformType::TEXTURE2D);

	mIrradianceMaps = mShader->createTextureUniform("irradianceMaps", UniformType::TEXTURE1D_ARRAY, 5);
	mReflectionMaps = mShader->createTextureUniform("reflectionMaps", UniformType::CUBE_MAP_ARRAY, 6);
	mBrdfLUT = mShader->createTextureUniform("brdfLUT", UniformType::TEXTURE2D, 7);
	mIrradianceArrayIndex = { mShader->getUniformLocation("irradianceArrayIndex"), UniformType::FLOAT };
	mReflectionArrayIndex = { mShader->getUniformLocation("reflectionArrayIndex"), UniformType::FLOAT };

	// shadow mapping
	mCascadedDepthMap = mShader->createTextureUniform("cascadedDepthMap", UniformType::TEXTURE2D_ARRAY, 8);




	mEyeLightDirection = { mShader->getUniformLocation("dirLight.directionEye"), UniformType::VEC4 };
	mLightDirectionWS = { mShader->getUniformLocation("dirLight.directionWorld"), UniformType::VEC4 };
	mLightColor = { mShader->getUniformLocation("dirLight.color"), UniformType::VEC4 };
	mLightPower = { mShader->getUniformLocation("dirLight.power"), UniformType::FLOAT };
	mAmbientLightPower = { mShader->getUniformLocation("ambientLightPower"), UniformType::FLOAT };
	mShadowStrength = { mShader->getUniformLocation("shadowStrength"), UniformType::FLOAT };

	mNearFarPlane = { mShader->getUniformLocation("nearFarPlane"), UniformType::VEC2 };

	SamplerDesc desc;
	//desc.minLOD = 0;
	//desc.maxLOD = 7;
	desc.minFilter = TexFilter::Linear_Mipmap_Linear;
	mReflectionSampler.setState(desc);
}

void PbrLightingData::setCascadedShadow(CascadedShadow* shadow)
{
	mCascadeShadow = shadow;
}

void PbrLightingData::updateConstants(const RenderContext& constants)
{
	setNearFarPlane(constants.camera->getNearFarPlaneViewSpace());

	if (mCascadeShadow) {
		setShadowStrength(mCascadeShadow->getShadowStrength());
		setCascadedDepthMap(mCascadeShadow->getDepthTextureArray());
	}

	if (mGlobalIllumination) {

		auto* probeManager = mGlobalIllumination->getProbeManager();
		auto* factory = probeManager->getFactory();
		auto* voxelConeTracer = mGlobalIllumination->getVoxelConeTracer();

		setBrdfLookupTexture(ProbeFactory::getBrdfLookupTexture());

		setIrradianceMaps(factory->getIrradianceSHMaps());
		setReflectionMaps(factory->getReflectionMaps());
		setIrradianceArrayIndex(probeManager->getDefaultIrradianceProbeID());
		setReflectionArrayIndex(probeManager->getDefaultReflectionProbeID());

		setAmbientLightPower(mGlobalIllumination->getAmbientPower());

		auto* envLightBuffer = probeManager->getEnvironmentLightShaderBuffer();
		envLightBuffer->bindToTarget(mEnvLightBindingPoint);

		auto* probeCluster = probeManager->getProbeCluster();
		auto* envLightCuller = probeCluster->getEnvLightCuller();

		probeCluster->getClusterAABBBuffer()->bindToTarget(mClustersAABBBindingPoint);
		envLightCuller->getGlobalLightIndexList()->bindToTarget(mEnvLightGlobalLightIndicesBindingPoint);
		envLightCuller->getLightGrids()->bindToTarget(mEnvLightLightGridsBindingPoint);

		mShader->setTexture(voxelConeTracer->getVoxelTexture(), &mReflectionSampler, 9);
		voxelConeTracer->getVoxelConstants()->bindToTarget(1);

		//mEnvLightGlobalLightIndicesBindingPoint
		//mEnvLightLightGridsBindingPoint
		//mClustersAABBBindingPoint
	}
}

void nex::PbrLightingData::updateLight(const DirLight& light, const Camera& camera)
{
	setLightColor(light.color);
	setLightPower(light.power);

	vec4 lightEyeDirection = camera.getView() * vec4(-vec3(light.directionWorld), 0.0f);
	setEyeLightDirection(lightEyeDirection);
	setLightDirectionWS(-light.directionWorld);
}

PbrForwardPass::PbrForwardPass(const ShaderFilePath& vertexShader, const ShaderFilePath& fragmentShader,
	GlobalIllumination* globalIllumination, CascadedShadow* cascadedShadow) :
	PbrGeometryShader(ShaderProgram::create(vertexShader, fragmentShader, nullptr, nullptr, nullptr, generateDefines(cascadedShadow)),
		TRANSFORM_BUFFER_BINDINGPOINT),
	mLightingPass(mProgram.get(), globalIllumination, cascadedShadow, PBR_PROBES_BUFFER_BINDINPOINT)
{
	/*mBiasMatrixSource = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);*/

	//bind();
	//mProgram->setMat4(mBiasMatrix.location, mBiasMatrixSource);
	//unbind();
}

void PbrForwardPass::updateConstants(const RenderContext& constants)
{
	bind();
	setViewProjectionMatrices(constants.camera->getProjectionMatrix(), 
		constants.camera->getView(), 
		constants.camera->getViewInv(),
		constants.camera->getViewPrev(), 
		constants.camera->getViewProjPrev());

	mGeometryData.updateConstants(constants);
	mLightingPass.updateConstants(constants);
}

void nex::PbrForwardPass::updateLight(const DirLight& light, const Camera & camera)
{
	bind();
	mLightingPass.updateLight(light, camera);
}

std::vector<std::string> PbrForwardPass::generateDefines(CascadedShadow* cascadedShadow)
{

	std::vector<std::string> vec;

	if (cascadedShadow) {
		vec = cascadedShadow->generateCsmDefines();
	}

	// csm CascadeBuffer and TransformBuffer both use binding point 0 per default. Resolve this conflict by using binding point 1 
	// for the TransformBuffer
	vec.push_back(std::string("#define PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT ") + std::to_string(TRANSFORM_BUFFER_BINDINGPOINT));

	// probes buffer must use another binding point, too.
	vec.push_back(std::string("#define PBR_PROBES_BUFFER_BINDING_POINT ") + std::to_string(PBR_PROBES_BUFFER_BINDINPOINT));

#ifdef USE_CLIP_SPACE_ZERO_TO_ONE
	vec.push_back("#define NDC_Z_ZERO_TO_ONE 1");
#else
	vec.push_back("#define NDC_Z_ZERO_TO_ONE 0");
#endif

	return vec;
}

PbrDeferredLightingPass::PbrDeferredLightingPass(const ShaderFilePath& vertexShader, const ShaderFilePath& fragmentShader, 
	GlobalIllumination* globalIllumination, CascadedShadow* cascadedShadow) :
	Shader(ShaderProgram::create(vertexShader, fragmentShader, nullptr, nullptr, nullptr, generateDefines(cascadedShadow))),
	mLightingPass(mProgram.get(), globalIllumination, cascadedShadow, PBR_PROBES_BUFFER_BINDINPOINT)
{
	mAlbedoMap = Shader::mProgram->createTextureUniform("gBuffer.albedoMap", UniformType::TEXTURE2D, 0);
	mAoMetalRoughnessMap = Shader::mProgram->createTextureUniform("gBuffer.aoMetalRoughnessMap", UniformType::TEXTURE2D, 1);
	mNormalEyeMap = Shader::mProgram->createTextureUniform("gBuffer.normalEyeMap", UniformType::TEXTURE2D, 2);
	mNormalizedViewSpaceZMap = Shader::mProgram->createTextureUniform("gBuffer.depthMap", UniformType::TEXTURE2D, 3);


	mIrradianceOutMap = Shader::mProgram->createTextureUniform("irradianceOutMap", UniformType::TEXTURE2D, PBR_IRRADIANCE_OUT_MAP_BINDINGPOINT);
	mAmbientReflectionOutMap = Shader::mProgram->createTextureUniform("ambientReflectionOutMap", UniformType::TEXTURE2D, PBR_AMBIENT_REFLECTION_OUT_MAP_BINDINGPOINT);
}

void PbrDeferredLightingPass::setAlbedoMap(const Texture* texture)
{
	Shader::mProgram->setTexture(texture, Sampler::getPoint(), mAlbedoMap.bindingSlot);
	//Shader::mProgram->setTextureByHandle(mAlbedoMap.location, texture);
}

void PbrDeferredLightingPass::setAoMetalRoughnessMap(const Texture* texture)
{
	//Shader::mProgram->setTextureByHandle(mAoMetalRoughnessMap.location, texture);
	Shader::mProgram->setTexture(texture, Sampler::getPoint(), mAoMetalRoughnessMap.bindingSlot);
}

void PbrDeferredLightingPass::setNormalEyeMap(const Texture* texture)
{
	//Shader::mProgram->setTextureByHandle(mNormalEyeMap.location, texture);
	Shader::mProgram->setTexture(texture, Sampler::getPoint(), mNormalEyeMap.bindingSlot);
}

void PbrDeferredLightingPass::setNormalizedViewSpaceZMap(const Texture* texture)
{
	//Shader::mProgram->setTextureByHandle(mNormalizedViewSpaceZMap.location, texture);
	Shader::mProgram->setTexture(texture, Sampler::getPoint(), mNormalizedViewSpaceZMap.bindingSlot);
}

void nex::PbrDeferredLightingPass::setIrradianceOutMap(const Texture* texture)
{
	Shader::mProgram->setTexture(texture, Sampler::getPoint(), mIrradianceOutMap.bindingSlot);
}

void nex::PbrDeferredLightingPass::setAmbientReflectionOutMap(const Texture* texture)
{
	Shader::mProgram->setTexture(texture, Sampler::getPoint(), mAmbientReflectionOutMap.bindingSlot);
}

void PbrDeferredLightingPass::updateConstants(const RenderContext& constants)
{
	bind();
	mLightingPass.updateConstants(constants);
}

void nex::PbrDeferredLightingPass::updateLight(const DirLight& light, const Camera & camera)
{
	bind();
	mLightingPass.updateLight(light, camera);
}

std::vector<std::string> nex::PbrDeferredLightingPass::generateDefines(CascadedShadow * cascadedShadow)
{
	std::vector<std::string> vec;
	if (cascadedShadow) {
		vec = cascadedShadow->generateCsmDefines();
	}

	vec.push_back(std::string("#define PBR_PROBES_BUFFER_BINDING_POINT ") + std::to_string(PBR_PROBES_BUFFER_BINDINPOINT));

#ifdef USE_CLIP_SPACE_ZERO_TO_ONE
	vec.push_back("#define NDC_Z_ZERO_TO_ONE 1");
#else
	vec.push_back("#define NDC_Z_ZERO_TO_ONE 0");
#endif

	return vec;
}


PbrDeferredGeometryShader::PbrDeferredGeometryShader(std::unique_ptr<ShaderProgram> shader) :
	PbrGeometryShader(std::move(shader))
{
}

void PbrDeferredGeometryShader::updateConstants(const RenderContext& constants)
{
	bind();
	setViewProjectionMatrices(constants.camera->getProjectionMatrix(), 
		constants.camera->getView(), 
		constants.camera->getViewInv(),
		constants.camera->getViewPrev(), 
		constants.camera->getViewProjPrev());
	mGeometryData.updateConstants(constants);
}

PbrConvolutionPass::PbrConvolutionPass()
{
	mProgram = ShaderProgram::create(
		"pbr/pbr_convolution_vs.glsl", "pbr/pbr_convolution_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };

	mEnvironmentMap = mProgram->createTextureUniform("environmentMap", UniformType::CUBE_MAP, 0);
}

void PbrConvolutionPass::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PbrConvolutionPass::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void PbrConvolutionPass::setEnvironmentMap(const CubeMap * cubeMap)
{
	mProgram->setTexture(cubeMap, &mSampler, mEnvironmentMap.bindingSlot);
}

nex::SHComputePass::SHComputePass() : ComputeShader(ShaderProgram::createComputeShader("pbr/probe/spherical_harmonics_from_cube_map_cs.glsl"))
{
	mEnvironmentMap = mProgram->createTextureUniform("environmentMaps", UniformType::CUBE_MAP, 0);
	mOutputMap = mProgram->createTextureUniform("result", UniformType::IMAGE1D_ARRAY, 1);

	mRowStart = { mProgram->getUniformLocation("rowStart"), UniformType::UINT};
}

void nex::SHComputePass::compute(Texture1DArray* texture, unsigned mipmap, const CubeMap* environmentMaps, unsigned rowStart, unsigned rowCount)
{
	mProgram->setTexture(environmentMaps, &mSampler, mEnvironmentMap.bindingSlot);

	mProgram->setImageLayerOfTexture(mOutputMap.location, texture, mOutputMap.bindingSlot, nex::TextureAccess::READ_WRITE,
		InternalFormat::RGBA32F, mipmap, false, 0);

	mProgram->setUInt(mRowStart.location, rowStart);

	// Each horizontal pixel is a SH coefficient. One row defines a set of sh coefficient for one environment map.
	this->dispatch(texture->getWidth(), rowCount, 1);
}

PbrPrefilterPass::PbrPrefilterPass()
{
	mProgram = ShaderProgram::create(
		"pbr/probe/pbr_prefilter_cubemap_vs.glsl", "pbr/probe/pbr_prefilter_cubemap_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mRoughness = { mProgram->getUniformLocation("roughness"), UniformType::FLOAT };

	//mEnvironmentMap = mProgram->createTextureUniform("environmentMap", UniformType::CUBE_MAP, 0);
	mEnvironmentMap = { mProgram->getUniformLocation("environmentMap"), UniformType::CUBE_MAP, 0 };
}

void PbrPrefilterPass::setMapToPrefilter(const CubeMap * cubeMap)
{
	mProgram->setTexture(cubeMap, Sampler::getLinear(), mEnvironmentMap.bindingSlot);
	mProgram->setBinding(mEnvironmentMap.location, mEnvironmentMap.bindingSlot);
}

void PbrPrefilterPass::setRoughness(float roughness)
{
	mProgram->setFloat(mRoughness.location, roughness);
}

void PbrPrefilterPass::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PbrPrefilterPass::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

PbrBrdfPrecomputePass::PbrBrdfPrecomputePass()
{
	mProgram = ShaderProgram::create(
		"screen_space_vs.glsl", "pbr/pbr_brdf_precompute_fs.glsl");
}

nex::PbrGeometryShader::PbrGeometryShader(std::unique_ptr<ShaderProgram> program, unsigned transformBindingPoint) :
	BasePbrGeometryShader(std::move(program), transformBindingPoint),
	mGeometryData(mProgram.get())
{
}

PbrGeometryData * nex::PbrGeometryShader::getShaderInterface()
{
	return &mGeometryData;
}

void nex::PbrGeometryShader::updateMaterial(const Material& material)
{
	const PbrMaterial* pbrMaterial;
	try {
		pbrMaterial = &dynamic_cast<const PbrMaterial&>(material);
	}
	catch (std::bad_cast & e) {
		throw_with_trace(e);
	}

	auto* shaderInterface = getShaderInterface();
	shaderInterface->setAlbedoMap(pbrMaterial->getAlbedoMap());
	shaderInterface->setAoMap(pbrMaterial->getAoMap());
	shaderInterface->setMetalMap(pbrMaterial->getMetallicMap());
	shaderInterface->setNormalMap(pbrMaterial->getNormalMap());
	shaderInterface->setRoughnessMap(pbrMaterial->getRoughnessMap());

	shaderInterface->setData(pbrMaterial->getData());
}

nex::PbrIrradianceShPass::PbrIrradianceShPass()
{
	mProgram = ShaderProgram::create(
		"pbr/probe/sh_irradiance_cubemap_vs.glsl", "pbr/probe/sh_irradiance_cubemap_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };

	mCoefficientMap = mProgram->createTextureUniform("coefficents", UniformType::TEXTURE1D, 0);

	mArrayIndex = { mProgram->getUniformLocation("arrayIndex"), UniformType::INT };

	mSampler.setMinFilter(TexFilter::Nearest);
	mSampler.setMagFilter(TexFilter::Nearest);
}

void nex::PbrIrradianceShPass::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void nex::PbrIrradianceShPass::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void nex::PbrIrradianceShPass::setCoefficientMap(const Texture1DArray* coefficients)
{
	mProgram->setTexture(coefficients, &mSampler, mCoefficientMap.bindingSlot);
}

void nex::PbrIrradianceShPass::setArrayIndex(int arrayIndex)
{
	mProgram->setInt(mArrayIndex.location, arrayIndex);
}

nex::PbrDeferredAmbientPass::PbrDeferredAmbientPass(GlobalIllumination* globalIllumination) : 
	Shader(ShaderProgram::create("screen_space_vs.glsl", "pbr/pbr_deferred_ambient_pass_fs.glsl", nullptr, nullptr, nullptr, 
		generateDefines(globalIllumination->getVoxelConeTracer()->isConeTracingUsed()))), 
	mGlobalIllumination(globalIllumination)
{
	mAlbedoMap = mProgram->createTextureUniform("gBuffer.albedoMap", UniformType::TEXTURE2D, PBR_ALBEDO_BINDINPOINT);
	mAoMetalRoughnessMap = mProgram->createTextureUniform("gBuffer.aoMetalRoughnessMap", UniformType::TEXTURE2D, PBR_AO_METAL_ROUGHNESS_BINDINPOINT);
	mNormalEyeMap = mProgram->createTextureUniform("gBuffer.normalEyeMap", UniformType::TEXTURE2D, PBR_NORMAL_BINDINPOINT);
	mDepthMap = mProgram->createTextureUniform("gBuffer.depthMap", UniformType::TEXTURE2D, PBR_DEPTH_BINDINPOINT);

	mIrradianceMaps = mProgram->createTextureUniform("irradianceMaps", UniformType::TEXTURE1D_ARRAY, PBR_IRRADIANCE_BINDING_POINT);
	mPrefilteredMaps = mProgram->createTextureUniform("prefilteredMaps", UniformType::CUBE_MAP_ARRAY, PBR_PREFILTERED_BINDING_POINT);
	mBrdfLUT = mProgram->createTextureUniform("brdfLUT", UniformType::TEXTURE2D, PBR_BRDF_LUT_BINDING_POINT);

	mVoxelTexture = mProgram->createTextureUniform("voxelTexture", UniformType::TEXTURE2D, VOXEL_TEXTURE_BINDING_POINT);


	mIrradianceArrayIndex = { mProgram->getUniformLocation("irradianceArrayIndex"), UniformType::FLOAT };
	mReflectionArrayIndex = { mProgram->getUniformLocation("reflectionArrayIndex"), UniformType::FLOAT };

	mAmbientLightPower = { mProgram->getUniformLocation("ambientLightPower"), UniformType::FLOAT };

	SamplerDesc desc;
	//desc.minLOD = 0;
	//desc.maxLOD = 7;
	desc.minFilter = TexFilter::Linear_Mipmap_Linear;
	mVoxelSampler.setState(desc);
}

void nex::PbrDeferredAmbientPass::setAlbedoMap(const Texture* texture)
{
	mProgram->setTexture(texture, Sampler::getPoint(), mAlbedoMap.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setAoMetalRoughnessMap(const Texture* texture)
{
	mProgram->setTexture(texture, Sampler::getPoint(), mAoMetalRoughnessMap.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setNormalEyeMap(const Texture* texture)
{
	mProgram->setTexture(texture, Sampler::getPoint(), mNormalEyeMap.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setDepthMap(const Texture* texture)
{
	mProgram->setTexture(texture, Sampler::getPoint(), mDepthMap.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setBrdfLookupTexture(const Texture* texture)
{
	mProgram->setTexture(texture, Sampler::getPoint(), mBrdfLUT.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setIrradianceMaps(const Texture* texture)
{
	mProgram->setTexture(texture, Sampler::getPoint(), mIrradianceMaps.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setPrefilteredMaps(const Texture* texture)
{
	mProgram->setTexture(texture, Sampler::getLinearMipMap(), mPrefilteredMaps.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setIrradianceArrayIndex(float index)
{
	mProgram->setFloat(mIrradianceArrayIndex.location, index);
}

void nex::PbrDeferredAmbientPass::setReflectionArrayIndex(float index)
{
	mProgram->setFloat(mReflectionArrayIndex.location, index);
}

void nex::PbrDeferredAmbientPass::setAmbientLightPower(float power)
{
	mProgram->setFloat(mAmbientLightPower.location, power);
}

void nex::PbrDeferredAmbientPass::updateConstants(const RenderContext& constants)
{
	bind();

	if (mGlobalIllumination) {

		auto* probeManager = mGlobalIllumination->getProbeManager();
		auto* factory = probeManager->getFactory();
		auto* voxelConeTracer = mGlobalIllumination->getVoxelConeTracer();

		setBrdfLookupTexture(ProbeFactory::getBrdfLookupTexture());

		setIrradianceMaps(factory->getIrradianceMaps());
		setPrefilteredMaps(factory->getReflectionMaps());
		setIrradianceArrayIndex(probeManager->getDefaultIrradianceProbeID());
		setReflectionArrayIndex(probeManager->getDefaultReflectionProbeID());

		setAmbientLightPower(mGlobalIllumination->getAmbientPower());

		auto* envLightBuffer = probeManager->getEnvironmentLightShaderBuffer();
		auto* probeCluster = probeManager->getProbeCluster();
		auto* envLightCuller = probeCluster->getEnvLightCuller();

		envLightBuffer->bindToTarget(PBR_PROBES_BUFFER_BINDINPOINT);
		probeCluster->getClusterAABBBuffer()->bindToTarget(PBR_CLUSTERS_AABB);
		envLightCuller->getGlobalLightIndexList()->bindToTarget(PBR_ENVIRONMENT_LIGHTS_GLOBAL_LIGHT_INDICES);
		envLightCuller->getLightGrids()->bindToTarget(PBR_ENVIRONMENT_LIGHTS_LIGHT_GRIDS);

		mProgram->setTexture(voxelConeTracer->getVoxelTexture(), &mVoxelSampler, mVoxelTexture.bindingSlot);
		voxelConeTracer->getVoxelConstants()->bindToTarget(VOXEL_C_UNIFORM_BUFFER_BINDING_POINT);
	}
}

std::vector<std::string> nex::PbrDeferredAmbientPass::generateDefines(bool useConeTracing)
{
	std::vector<std::string> vec;
	
	vec.push_back(std::string("#define PBR_ALBEDO_BINDINPOINT ") + std::to_string(PBR_ALBEDO_BINDINPOINT));
	vec.push_back(std::string("#define PBR_AO_METAL_ROUGHNESS_BINDINPOINT ") + std::to_string(PBR_AO_METAL_ROUGHNESS_BINDINPOINT));
	vec.push_back(std::string("#define PBR_NORMAL_BINDINPOINT ") + std::to_string(PBR_NORMAL_BINDINPOINT));
	vec.push_back(std::string("#define PBR_DEPTH_BINDINPOINT ") + std::to_string(PBR_DEPTH_BINDINPOINT));
	vec.push_back(std::string("#define PBR_IRRADIANCE_BINDING_POINT ") + std::to_string(PBR_IRRADIANCE_BINDING_POINT));
	vec.push_back(std::string("#define PBR_PREFILTERED_BINDING_POINT ") + std::to_string(PBR_PREFILTERED_BINDING_POINT));
	vec.push_back(std::string("#define PBR_BRDF_LUT_BINDING_POINT ") + std::to_string(PBR_BRDF_LUT_BINDING_POINT));
	vec.push_back(std::string("#define VOXEL_TEXTURE_BINDING_POINT ") + std::to_string(VOXEL_TEXTURE_BINDING_POINT));

	vec.push_back(std::string("#define VOXEL_C_UNIFORM_BUFFER_BINDING_POINT ") + std::to_string(VOXEL_C_UNIFORM_BUFFER_BINDING_POINT));

	
	vec.push_back(std::string("#define PBR_PROBES_BUFFER_BINDINPOINT ") + std::to_string(PBR_PROBES_BUFFER_BINDINPOINT));
	vec.push_back(std::string("#define PBR_ENVIRONMENT_LIGHTS_GLOBAL_LIGHT_INDICES ") + std::to_string(PBR_ENVIRONMENT_LIGHTS_GLOBAL_LIGHT_INDICES));
	vec.push_back(std::string("#define PBR_ENVIRONMENT_LIGHTS_LIGHT_GRIDS ") + std::to_string(PBR_ENVIRONMENT_LIGHTS_LIGHT_GRIDS));
	vec.push_back(std::string("#define PBR_CLUSTERS_AABB ") + std::to_string(PBR_CLUSTERS_AABB));



	if (useConeTracing)
		vec.push_back(std::string("#define USE_CONE_TRACING 1"));
	else {
		vec.push_back(std::string("#define USE_CONE_TRACING 0"));
	}

#ifdef USE_CLIP_SPACE_ZERO_TO_ONE
	vec.push_back("#define NDC_Z_ZERO_TO_ONE 1");
#else
	vec.push_back("#define NDC_Z_ZERO_TO_ONE 0");
#endif

	return vec;
}

nex::PbrGeometryBonesData::PbrGeometryBonesData(ShaderProgram* shader, unsigned bonesBufferBindingPoint) : 
	PbrGeometryData(shader), mBonesBufferBindingPoint(bonesBufferBindingPoint)
{
}

void nex::PbrGeometryBonesData::bindBonesBuffer(ShaderStorageBuffer* buffer)
{
	buffer->bindToTarget(mBonesBufferBindingPoint);
}

nex::PbrDeferredGeometryBonesShader::PbrDeferredGeometryBonesShader(std::unique_ptr<ShaderProgram> shader) : 
	PbrGeometryBonesShader(std::move(shader))
{
}

void nex::PbrDeferredGeometryBonesShader::updateConstants(const RenderContext& constants)
{
	bind();
	setViewProjectionMatrices(constants.camera->getProjectionMatrix(), 
		constants.camera->getView(), 
		constants.camera->getViewInv(),
		constants.camera->getViewPrev(), 
		constants.camera->getViewProjPrev());

	mGeometryBonesData.updateConstants(constants);
}

nex::PbrGeometryBonesShader::PbrGeometryBonesShader(std::unique_ptr<ShaderProgram> program, 
	unsigned transformBindingPoint, 
	unsigned bonesBufferBindinPoint) : BasePbrGeometryShader(std::move(program), transformBindingPoint),
	mGeometryBonesData(mProgram.get(), bonesBufferBindinPoint)
{
}

PbrGeometryBonesData* nex::PbrGeometryBonesShader::getShaderInterface()
{
	return &mGeometryBonesData;
}

void nex::PbrGeometryBonesShader::updateMaterial(const Material& material)
{
	const PbrMaterial* pbrMaterial;
	try {
		pbrMaterial = &dynamic_cast<const PbrMaterial&>(material);
	}
	catch (std::bad_cast & e) {
		throw_with_trace(e);
	}


	auto* shaderInterface = getShaderInterface();
	shaderInterface->setAlbedoMap(pbrMaterial->getAlbedoMap());
	shaderInterface->setAoMap(pbrMaterial->getAoMap());
	shaderInterface->setMetalMap(pbrMaterial->getMetallicMap());
	shaderInterface->setNormalMap(pbrMaterial->getNormalMap());
	shaderInterface->setRoughnessMap(pbrMaterial->getRoughnessMap());
}

nex::BasePbrGeometryShader::BasePbrGeometryShader(std::unique_ptr<ShaderProgram> program, unsigned transformBindingPoint) : 
	TransformShader(std::move(program), transformBindingPoint)
{
}
