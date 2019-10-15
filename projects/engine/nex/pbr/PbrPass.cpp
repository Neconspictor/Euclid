#include <nex/pbr/PbrPass.hpp>
#include <glm/glm.hpp>
#include <nex/mesh/Mesh.hpp>
#include <nex/material/Material.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nex/texture/TextureManager.hpp"
#include "nex/pbr/PbrProbe.hpp"
#include <nex/pbr/GlobalIllumination.hpp>
#include <nex/pbr/Cluster.hpp>

using namespace glm;
using namespace std;
using namespace nex;

PbrGeometryData::PbrGeometryData(Shader* shader) : PbrBaseCommon(shader),
mDefaultImageSampler(nullptr)
{
	assert(mShader != nullptr);

	// mesh material
	mAlbedoMap = mShader->createTextureUniform("material.albedoMap", UniformType::TEXTURE2D, ALBEDO_BINDING_POINT);
	mAmbientOcclusionMap = mShader->createTextureUniform("material.aoMap", UniformType::TEXTURE2D, AO_BINDING_POINT);
	mMetalMap = mShader->createTextureUniform("material.metallicMap", UniformType::TEXTURE2D, METALLIC_BINDING_POINT);
	mNormalMap = mShader->createTextureUniform("material.normalMap", UniformType::TEXTURE2D, NORMAL_BINDING_POINT);
	mRoughnessMap = mShader->createTextureUniform("material.roughnessMap", UniformType::TEXTURE2D, ROUGHNESS_BINDING_POINT);

	mNearFarPlane = { mShader->getUniformLocation("nearFarPlane"), UniformType::VEC2 };

	mDefaultImageSampler = TextureManager::get()->getDefaultImageSampler();
}

void nex::PbrGeometryData::setAlbedoMap(const Texture* albedo)
{
	mShader->setTexture(albedo, mDefaultImageSampler, mAlbedoMap.bindingSlot);
}

void nex::PbrGeometryData::setAoMap(const Texture* ao)
{
	mShader->setTexture(ao, mDefaultImageSampler, mAmbientOcclusionMap.bindingSlot);
}

void nex::PbrGeometryData::setMetalMap(const Texture* metal)
{
	mShader->setTexture(metal, mDefaultImageSampler, mMetalMap.bindingSlot);
}

void nex::PbrGeometryData::setNormalMap(const Texture* normal)
{
	mShader->setTexture(normal, mDefaultImageSampler, mNormalMap.bindingSlot);
}

void nex::PbrGeometryData::setRoughnessMap(const Texture* roughness)
{
	mShader->setTexture(roughness, mDefaultImageSampler, mRoughnessMap.bindingSlot);
}

void PbrGeometryData::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mShader->setVec2(mNearFarPlane.location, nearFarPlane);
}

PbrBaseCommon::PbrBaseCommon(Shader* shader) : mShader(shader)
{
}

void PbrBaseCommon::setShader(Shader* shader)
{
	mShader = shader;
}

void PbrGeometryData::updateConstants(const Pass::Constants& constants)
{
	/*mDefaultImageSampler->bind(ALBEDO_BINDING_POINT);
	mDefaultImageSampler->bind(AO_BINDING_POINT);
	mDefaultImageSampler->bind(METALLIC_BINDING_POINT);
	mDefaultImageSampler->bind(NORMAL_BINDING_POINT);
	mDefaultImageSampler->bind(ROUGHNESS_BINDING_POINT);*/

	setNearFarPlane(constants.camera->getNearFarPlaneViewSpace());
}

void nex::PbrLightingData::setArrayIndex(float index)
{
	mShader->setFloat(mArrayIndex.location, index);
}

void PbrLightingData::setBrdfLookupTexture(const Texture* brdfLUT)
{
	//mShader->setTextureByHandle(mBrdfLUT.location, brdfLUT);
	mShader->setTexture(brdfLUT, &mSampler, mBrdfLUT.bindingSlot);
}

void nex::PbrLightingData::setIrradianceMaps(const CubeMapArray * texture)
{
	mShader->setTexture(texture, &mSampler, mIrradianceMaps.bindingSlot);
}

void nex::PbrLightingData::setPrefilteredMaps(const CubeMapArray * texture)
{
	mShader->setTexture(texture, &mPrefilteredSampler, mPrefilteredMaps.bindingSlot);
}

void PbrLightingData::setCascadedDepthMap(const Texture* cascadedDepthMap)
{
	mShader->setTexture(cascadedDepthMap, &mCascadedShadowMapSampler, mCascadedDepthMap.bindingSlot);
	//mShader->setTextureByHandle(mCascadedDepthMap.location, cascadedDepthMap);
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

void nex::PbrLightingData::setLightDirectionWS(const glm::vec3& direction)
{
	mShader->setVec3(mLightDirectionWS.location, direction);
}

void PbrLightingData::setEyeLightDirection(const glm::vec3& direction)
{
	mShader->setVec3(mEyeLightDirection.location, direction);
}

void PbrLightingData::setLightColor(const glm::vec3& color)
{
	mShader->setVec3(mLightColor.location, color);
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

void PbrLightingData::setInverseViewMatrix(const glm::mat4& mat)
{
	mShader->setMat4(mInverseView.location, mat);
}

void PbrLightingData::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mShader->setVec2(mNearFarPlane.location, nearFarPlane);
}

nex::PbrLightingData::PbrLightingData(Shader * shader, GlobalIllumination* globalIllumination, 
	CascadedShadow* cascadedShadow, unsigned csmCascadeBindingPoint, unsigned envLightBindingPoint,
	unsigned envLightGlobalLightIndicesBindingPoint,
	unsigned envLightLightGridsBindingPoint,
	unsigned clustersAABBBindingPoint,
	unsigned constantsBindingPoint) :
	PbrBaseCommon(shader),
	mEnvLightBindingPoint(envLightBindingPoint),
	mGlobalIllumination(globalIllumination),
	mCsmCascadeBindingPoint(csmCascadeBindingPoint),
	
	//cascadeBufferUBO(
	//	CSM_CASCADE_BUFFER_BINDING_POINT, 
	//	CascadedShadow::CascadeData::calcCascadeDataByteSize(cascadedShadow->getCascadeData().numCascades),
	//	ShaderBuffer::UsageHint::DYNAMIC_COPY), mProbe(nullptr),
	mCascadeShadow(cascadedShadow),
	mEnvLightGlobalLightIndicesBindingPoint(envLightGlobalLightIndicesBindingPoint),
	mEnvLightLightGridsBindingPoint(envLightLightGridsBindingPoint),
	mClustersAABBBindingPoint(clustersAABBBindingPoint),
	mConstantsBindingPoint(constantsBindingPoint),
	mConstantsBuffer(mConstantsBindingPoint,sizeof(PbrConstants), nullptr, GpuBuffer::UsageHint::DYNAMIC_DRAW)
{
	assert(mShader != nullptr);

	mCascadedShadowMapSampler.setMinFilter(TextureFilter::NearestNeighbor);
	mCascadedShadowMapSampler.setMagFilter(TextureFilter::NearestNeighbor);

	// ibl
	//mIrradianceMap = mShader->createTextureHandleUniform("irradianceMap", UniformType::CUBE_MAP);
	//mPrefilterMap = mShader->createTextureHandleUniform("prefilterMap", UniformType::CUBE_MAP);
	//mBrdfLUT = mShader->createTextureHandleUniform("brdfLUT", UniformType::TEXTURE2D);

	mIrradianceMaps = mShader->createTextureUniform("irradianceMaps", UniformType::CUBE_MAP_ARRAY, 5);
	mPrefilteredMaps = mShader->createTextureUniform("prefilteredMaps", UniformType::CUBE_MAP_ARRAY, 6);
	mBrdfLUT = mShader->createTextureUniform("brdfLUT", UniformType::TEXTURE2D, 7);
	mArrayIndex = { mShader->getUniformLocation("arrayIndex"), UniformType::FLOAT };

	// shaodw mapping
	mCascadedDepthMap = mShader->createTextureUniform("cascadedDepthMap", UniformType::TEXTURE2D_ARRAY, 8);




	mEyeLightDirection = { mShader->getUniformLocation("dirLight.directionEye"), UniformType::VEC3 };
	mLightDirectionWS = { mShader->getUniformLocation("dirLight.directionWorld"), UniformType::VEC3 };
	mLightColor = { mShader->getUniformLocation("dirLight.color"), UniformType::VEC3 };
	mLightPower = { mShader->getUniformLocation("dirLight.power"), UniformType::FLOAT };
	mAmbientLightPower = { mShader->getUniformLocation("ambientLightPower"), UniformType::FLOAT };
	mShadowStrength = { mShader->getUniformLocation("shadowStrength"), UniformType::FLOAT };

	mInverseView = { mShader->getUniformLocation("inverseViewMatrix"), UniformType::MAT4 };

	mNearFarPlane = { mShader->getUniformLocation("nearFarPlane"), UniformType::VEC2 };

	SamplerDesc desc;
	//desc.minLOD = 0;
	//desc.maxLOD = 7;
	desc.minFilter = TextureFilter::Linear_Mipmap_Linear;
	mPrefilteredSampler.setState(desc);
}

void PbrLightingData::setCascadedShadow(CascadedShadow* shadow)
{
	mCascadeShadow = shadow;
}

void PbrLightingData::updateConstants(const Pass::Constants& constants)
{
	setInverseViewMatrix(inverse(constants.camera->getView()));

	setNearFarPlane(constants.camera->getNearFarPlaneViewSpace());

	PbrConstants pbrConstants;
	pbrConstants.windowDimension = glm::uvec4(constants.windowWidth, constants.windowHeight, 0, 0);
	pbrConstants.clusterDimension = glm::uvec4(16,8,24,0);
	pbrConstants.nearFarDistance = glm::vec4(constants.camera->getNearDistance(), constants.camera->getFarDistance(), 0, 0);
	mConstantsBuffer.update(sizeof(PbrConstants), &pbrConstants);
	mConstantsBuffer.bindToTarget(mConstantsBindingPoint);

	if (mCascadeShadow) {
		setShadowStrength(mCascadeShadow->getShadowStrength());
		auto* buffer = mCascadeShadow->getCascadeBuffer();
		buffer->bindToTarget(mCsmCascadeBindingPoint);
		setCascadedDepthMap(mCascadeShadow->getDepthTextureArray());
	}

	if (mGlobalIllumination) {
		setBrdfLookupTexture(PbrProbe::getBrdfLookupTexture());

		setIrradianceMaps(mGlobalIllumination->getIrradianceMaps());
		setPrefilteredMaps(mGlobalIllumination->getPrefilteredMaps());

		setAmbientLightPower(mGlobalIllumination->getAmbientPower());

		auto* envLightBuffer = mGlobalIllumination->getEnvironmentLightShaderBuffer();
		envLightBuffer->bindToTarget(mEnvLightBindingPoint);

		auto* probeCluster = mGlobalIllumination->getProbeCluster();
		auto* envLightCuller = probeCluster->getEnvLightCuller();

		probeCluster->getClusterAABBBuffer()->bindToTarget(mClustersAABBBindingPoint);
		envLightCuller->getGlobalLightIndexList()->bindToTarget(mEnvLightGlobalLightIndicesBindingPoint);
		envLightCuller->getLightGrids()->bindToTarget(mEnvLightLightGridsBindingPoint);

		mShader->setTexture(mGlobalIllumination->getVoxelTexture(), &mPrefilteredSampler, 9);
		mGlobalIllumination->getVoxelConstants()->bindToTarget(1);

		//mEnvLightGlobalLightIndicesBindingPoint
		//mEnvLightLightGridsBindingPoint
		//mClustersAABBBindingPoint
	}
}

void nex::PbrLightingData::updateLight(const DirLight& light, const Camera& camera)
{
	setLightColor(light.color);
	setLightPower(light.power);

	vec4 lightEyeDirection = camera.getView() * vec4(-light.directionWorld, 0.0f);
	setEyeLightDirection(vec3(lightEyeDirection));
	setLightDirectionWS(-light.directionWorld);
}

PbrForwardPass::PbrForwardPass(const ShaderFilePath& vertexShader, const ShaderFilePath& fragmentShader,
	GlobalIllumination* globalIllumination, CascadedShadow* cascadedShadow) :
	PbrGeometryPass(Shader::create(vertexShader, fragmentShader, nullptr, nullptr, nullptr, generateDefines(cascadedShadow)),
		TRANSFORM_BUFFER_BINDINGPOINT),
	mLightingPass(mShader.get(), globalIllumination, cascadedShadow, CASCADE_BUFFER_BINDINGPOINT, PBR_PROBES_BUFFER_BINDINPOINT)
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

void PbrForwardPass::updateConstants(const Pass::Constants& constants)
{
	bind();
	setViewProjectionMatrices(constants.camera->getProjectionMatrix(), constants.camera->getView(), constants.camera->getViewPrev(), constants.camera->getViewProjPrev());

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
	vec.push_back(std::string("#define CSM_CASCADE_BUFFER_BINDING_POINT ") + std::to_string(CASCADE_BUFFER_BINDINGPOINT));
	vec.push_back(std::string("#define PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT ") + std::to_string(TRANSFORM_BUFFER_BINDINGPOINT));

	// probes buffer must use another binding point, too.
	vec.push_back(std::string("#define PBR_PROBES_BUFFER_BINDING_POINT ") + std::to_string(PBR_PROBES_BUFFER_BINDINPOINT));

	return vec;
}

PbrDeferredLightingPass::PbrDeferredLightingPass(const ShaderFilePath& vertexShader, const ShaderFilePath& fragmentShader, 
	GlobalIllumination* globalIllumination, CascadedShadow* cascadedShadow) :
	Pass(Shader::create(vertexShader, fragmentShader, nullptr, nullptr, nullptr, generateDefines(cascadedShadow))),
	mLightingPass(mShader.get(), globalIllumination, cascadedShadow, CASCADE_BUFFER_BINDINGPOINT, PBR_PROBES_BUFFER_BINDINPOINT)
{
	mAlbedoMap = Pass::mShader->createTextureUniform("gBuffer.albedoMap", UniformType::TEXTURE2D, 0);
	mAoMetalRoughnessMap = Pass::mShader->createTextureUniform("gBuffer.aoMetalRoughnessMap", UniformType::TEXTURE2D, 1);
	mNormalEyeMap = Pass::mShader->createTextureUniform("gBuffer.normalEyeMap", UniformType::TEXTURE2D, 2);
	mNormalizedViewSpaceZMap = Pass::mShader->createTextureUniform("gBuffer.depthMap", UniformType::TEXTURE2D, 3);


	mIrradianceOutMap = Pass::mShader->createTextureUniform("irradianceOutMap", UniformType::TEXTURE2D, PBR_IRRADIANCE_OUT_MAP_BINDINGPOINT);
	mAmbientReflectionOutMap = Pass::mShader->createTextureUniform("ambientReflectionOutMap", UniformType::TEXTURE2D, PBR_AMBIENT_REFLECTION_OUT_MAP_BINDINGPOINT);

	mInverseProjFromGPass = { Pass::mShader->getUniformLocation("inverseProjMatrix_GPass"), UniformType::MAT4 };

	auto state = Pass::mSampler.getState();
	state.wrapR = state.wrapS = state.wrapT = TextureUVTechnique::ClampToEdge;
	state.minFilter = state.magFilter = TextureFilter::NearestNeighbor;
	Pass::mSampler.setState(state);
}

void PbrDeferredLightingPass::setAlbedoMap(const Texture* texture)
{
	Pass::mShader->setTexture(texture, &(Pass::mSampler), mAlbedoMap.bindingSlot);
	//Pass::mShader->setTextureByHandle(mAlbedoMap.location, texture);
}

void PbrDeferredLightingPass::setAoMetalRoughnessMap(const Texture* texture)
{
	//Pass::mShader->setTextureByHandle(mAoMetalRoughnessMap.location, texture);
	Pass::mShader->setTexture(texture, &(Pass::mSampler), mAoMetalRoughnessMap.bindingSlot);
}

void PbrDeferredLightingPass::setNormalEyeMap(const Texture* texture)
{
	//Pass::mShader->setTextureByHandle(mNormalEyeMap.location, texture);
	Pass::mShader->setTexture(texture, &(Pass::mSampler), mNormalEyeMap.bindingSlot);
}

void PbrDeferredLightingPass::setNormalizedViewSpaceZMap(const Texture* texture)
{
	//Pass::mShader->setTextureByHandle(mNormalizedViewSpaceZMap.location, texture);
	Pass::mShader->setTexture(texture, &(Pass::mSampler), mNormalizedViewSpaceZMap.bindingSlot);
}

void nex::PbrDeferredLightingPass::setIrradianceOutMap(const Texture* texture)
{
	Pass::mShader->setTexture(texture, &(Pass::mSampler), mIrradianceOutMap.bindingSlot);
}

void nex::PbrDeferredLightingPass::setAmbientReflectionOutMap(const Texture* texture)
{
	Pass::mShader->setTexture(texture, &(Pass::mSampler), mAmbientReflectionOutMap.bindingSlot);
}

void PbrDeferredLightingPass::setInverseProjMatrixFromGPass(const glm::mat4& mat)
{
	Pass::mShader->setMat4(mInverseProjFromGPass.location, mat);
}

void PbrDeferredLightingPass::updateConstants(const Pass::Constants& constants)
{
	bind();
	mLightingPass.updateConstants(constants);
	setInverseProjMatrixFromGPass(inverse(constants.camera->getProjectionMatrix()));
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

	vec.push_back(std::string("#define CSM_CASCADE_BUFFER_BINDING_POINT ") + std::to_string(CASCADE_BUFFER_BINDINGPOINT));
	vec.push_back(std::string("#define PBR_PROBES_BUFFER_BINDING_POINT ") + std::to_string(PBR_PROBES_BUFFER_BINDINPOINT));

	return vec;
}


PbrDeferredGeometryPass::PbrDeferredGeometryPass(std::unique_ptr<Shader> shader) :
	PbrGeometryPass(std::move(shader))
{
}

void PbrDeferredGeometryPass::updateConstants(const Pass::Constants& constants)
{
	bind();
	setViewProjectionMatrices(constants.camera->getProjectionMatrix(), constants.camera->getView(), constants.camera->getViewPrev(), constants.camera->getViewProjPrev());
	mGeometryData.updateConstants(constants);
}

PbrConvolutionPass::PbrConvolutionPass()
{
	mShader = Shader::create(
		"pbr/pbr_convolution_vs.glsl", "pbr/pbr_convolution_fs.glsl");

	mProjection = { mShader->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mShader->getUniformLocation("view"), UniformType::MAT4 };

	mEnvironmentMap = mShader->createTextureUniform("environmentMap", UniformType::CUBE_MAP, 0);
}

void PbrConvolutionPass::setProjection(const glm::mat4& mat)
{
	mShader->setMat4(mProjection.location, mat);
}

void PbrConvolutionPass::setView(const glm::mat4& mat)
{
	mShader->setMat4(mView.location, mat);
}

void PbrConvolutionPass::setEnvironmentMap(const CubeMap * cubeMap)
{
	mShader->setTexture(cubeMap, &mSampler, mEnvironmentMap.bindingSlot);
}

nex::SHComputePass::SHComputePass() : ComputePass(Shader::createComputeShader("pbr/probe/spherical_harmonics_from_cube_map_cs.glsl"))
{
	mEnvironmentMap = mShader->createTextureUniform("environmentMaps", UniformType::CUBE_MAP, 0);
	mOutputMap = mShader->createTextureUniform("result", UniformType::IMAGE2D, 1);

	mRowStart = { mShader->getUniformLocation("rowStart"), UniformType::UINT};
}

void nex::SHComputePass::compute(Texture2D* texture, unsigned mipmap, const CubeMap* environmentMaps, unsigned rowStart, unsigned rowCount)
{
	mShader->setTexture(environmentMaps, &mSampler, mEnvironmentMap.bindingSlot);

	mShader->setImageLayerOfTexture(mOutputMap.location, texture, mOutputMap.bindingSlot, nex::TextureAccess::READ_WRITE,
		InternFormat::RGBA32F, mipmap, false, 0);

	mShader->setUInt(mRowStart.location, rowStart);

	// Each horizontal pixel is a SH coefficient. One row defines a set of sh coefficient for one environment map.
	this->dispatch(texture->getWidth(), rowCount, 1);
}

PbrPrefilterPass::PbrPrefilterPass()
{
	mShader = Shader::create(
		"pbr/pbr_prefilter_cubemap_vs.glsl", "pbr/pbr_prefilter_cubemap_fs.glsl");

	mProjection = { mShader->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mShader->getUniformLocation("view"), UniformType::MAT4 };
	mRoughness = { mShader->getUniformLocation("roughness"), UniformType::FLOAT };

	//mEnvironmentMap = mProgram->createTextureUniform("environmentMap", UniformType::CUBE_MAP, 0);
	mEnvironmentMap = { mShader->getUniformLocation("environmentMap"), UniformType::CUBE_MAP, 0 };
}

void PbrPrefilterPass::setMapToPrefilter(CubeMap * cubeMap)
{
	mShader->setTexture(cubeMap, &mSampler, mEnvironmentMap.bindingSlot);
	mShader->setBinding(mEnvironmentMap.location, mEnvironmentMap.bindingSlot);
}

void PbrPrefilterPass::setRoughness(float roughness)
{
	mShader->setFloat(mRoughness.location, roughness);
}

void PbrPrefilterPass::setProjection(const glm::mat4& mat)
{
	mShader->setMat4(mProjection.location, mat);
}

void PbrPrefilterPass::setView(const glm::mat4& mat)
{
	mShader->setMat4(mView.location, mat);
}

PbrBrdfPrecomputePass::PbrBrdfPrecomputePass()
{
	mShader = Shader::create(
		"screen_space_vs.glsl", "pbr/pbr_brdf_precompute_fs.glsl");
}

nex::PbrGeometryPass::PbrGeometryPass(std::unique_ptr<Shader> program, unsigned transformBindingPoint) :
	TransformPass(std::move(program), transformBindingPoint),
	mGeometryData(mShader.get())
{
}

PbrGeometryData * nex::PbrGeometryPass::getShaderInterface()
{
	return &mGeometryData;
}

nex::PbrIrradianceShPass::PbrIrradianceShPass()
{
	mShader = Shader::create(
		"pbr/probe/sh_irradiance_cubemap_vs.glsl", "pbr/probe/sh_irradiance_cubemap_fs.glsl");

	mProjection = { mShader->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mShader->getUniformLocation("view"), UniformType::MAT4 };

	mCoefficientMap = mShader->createTextureUniform("coefficents", UniformType::TEXTURE2D, 0);
	mSampler.setMinFilter(TextureFilter::NearestNeighbor);
	mSampler.setMagFilter(TextureFilter::NearestNeighbor);
}

void nex::PbrIrradianceShPass::setProjection(const glm::mat4& mat)
{
	mShader->setMat4(mProjection.location, mat);
}

void nex::PbrIrradianceShPass::setView(const glm::mat4& mat)
{
	mShader->setMat4(mView.location, mat);
}

void nex::PbrIrradianceShPass::setCoefficientMap(const Texture2D* coefficients)
{
	mShader->setTexture(coefficients, &mSampler, mCoefficientMap.bindingSlot);
}

nex::PbrDeferredAmbientPass::PbrDeferredAmbientPass(GlobalIllumination* globalIllumination) : 
	Pass(Shader::create("screen_space_vs.glsl", "pbr/pbr_deferred_ambient_pass_fs.glsl", nullptr, nullptr, nullptr, 
		generateDefines(globalIllumination->isConeTracingUsed()))), 
	mGlobalIllumination(globalIllumination),
	mConstantsBuffer(PBR_CONSTANTS, sizeof(PbrConstants), nullptr, GpuBuffer::UsageHint::DYNAMIC_DRAW)
{
	mAlbedoMap = mShader->createTextureUniform("gBuffer.albedoMap", UniformType::TEXTURE2D, PBR_ALBEDO_BINDINPOINT);
	mAoMetalRoughnessMap = mShader->createTextureUniform("gBuffer.aoMetalRoughnessMap", UniformType::TEXTURE2D, PBR_AO_METAL_ROUGHNESS_BINDINPOINT);
	mNormalEyeMap = mShader->createTextureUniform("gBuffer.normalEyeMap", UniformType::TEXTURE2D, PBR_NORMAL_BINDINPOINT);
	mDepthMap = mShader->createTextureUniform("gBuffer.depthMap", UniformType::TEXTURE2D, PBR_DEPTH_BINDINPOINT);

	mIrradianceMaps = mShader->createTextureUniform("irradianceMaps", UniformType::CUBE_MAP_ARRAY, PBR_IRRADIANCE_BINDING_POINT);
	mPrefilteredMaps = mShader->createTextureUniform("prefilteredMaps", UniformType::CUBE_MAP_ARRAY, PBR_PREFILTERED_BINDING_POINT);
	mBrdfLUT = mShader->createTextureUniform("brdfLUT", UniformType::TEXTURE2D, PBR_BRDF_LUT_BINDING_POINT);

	mVoxelTexture = mShader->createTextureUniform("voxelTexture", UniformType::TEXTURE2D, VOXEL_TEXTURE_BINDING_POINT);


	mArrayIndex = { mShader->getUniformLocation("arrayIndex"), UniformType::FLOAT };

	mInverseProjFromGPass = { mShader->getUniformLocation("inverseProjMatrix_GPass"), UniformType::MAT4 };
	mAmbientLightPower = { mShader->getUniformLocation("ambientLightPower"), UniformType::FLOAT };
	mInverseView = { mShader->getUniformLocation("inverseViewMatrix"), UniformType::MAT4 };

	auto state = mSampler.getState();
	state.wrapR = state.wrapS = state.wrapT = TextureUVTechnique::ClampToEdge;
	state.minFilter = state.magFilter = TextureFilter::NearestNeighbor;
	mSampler.setState(state);

	SamplerDesc desc;
	//desc.minLOD = 0;
	//desc.maxLOD = 7;
	desc.minFilter = TextureFilter::Linear_Mipmap_Linear;
	mVoxelSampler.setState(desc);
}

void nex::PbrDeferredAmbientPass::setAlbedoMap(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mAlbedoMap.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setAoMetalRoughnessMap(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mAoMetalRoughnessMap.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setNormalEyeMap(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mNormalEyeMap.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setDepthMap(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mDepthMap.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setBrdfLookupTexture(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mBrdfLUT.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setIrradianceMaps(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mIrradianceMaps.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setPrefilteredMaps(const Texture* texture)
{
	mShader->setTexture(texture, &mSampler, mPrefilteredMaps.bindingSlot);
}

void nex::PbrDeferredAmbientPass::setAmbientLightPower(float power)
{
	mShader->setFloat(mAmbientLightPower.location, power);
}

void nex::PbrDeferredAmbientPass::setInverseViewMatrix(const glm::mat4& mat)
{
	mShader->setMat4(mInverseView.location, mat);
}

void nex::PbrDeferredAmbientPass::setInverseProjMatrixFromGPass(const glm::mat4& mat)
{
	mShader->setMat4(mInverseProjFromGPass.location, mat);
}

void nex::PbrDeferredAmbientPass::updateConstants(const Constants& constants)
{
	bind();
	setInverseViewMatrix(inverse(constants.camera->getView()));
	setInverseProjMatrixFromGPass(inverse(constants.camera->getProjectionMatrix()));

	if (mGlobalIllumination) {
		setBrdfLookupTexture(PbrProbe::getBrdfLookupTexture());

		setIrradianceMaps(mGlobalIllumination->getIrradianceMaps());
		setPrefilteredMaps(mGlobalIllumination->getPrefilteredMaps());

		setAmbientLightPower(mGlobalIllumination->getAmbientPower());

		auto* envLightBuffer = mGlobalIllumination->getEnvironmentLightShaderBuffer();
		auto* probeCluster = mGlobalIllumination->getProbeCluster();
		auto* envLightCuller = probeCluster->getEnvLightCuller();

		envLightBuffer->bindToTarget(PBR_PROBES_BUFFER_BINDINPOINT);
		probeCluster->getClusterAABBBuffer()->bindToTarget(PBR_CLUSTERS_AABB);
		envLightCuller->getGlobalLightIndexList()->bindToTarget(PBR_ENVIRONMENT_LIGHTS_GLOBAL_LIGHT_INDICES);
		envLightCuller->getLightGrids()->bindToTarget(PBR_ENVIRONMENT_LIGHTS_LIGHT_GRIDS);

		mShader->setTexture(mGlobalIllumination->getVoxelTexture(), &mVoxelSampler, mVoxelTexture.bindingSlot);
		mGlobalIllumination->getVoxelConstants()->bindToTarget(VOXEL_C_UNIFORM_BUFFER_BINDING_POINT);
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

	vec.push_back(std::string("#define PBR_CONSTANTS ") + std::to_string(PBR_CONSTANTS));
	vec.push_back(std::string("#define VOXEL_C_UNIFORM_BUFFER_BINDING_POINT ") + std::to_string(VOXEL_C_UNIFORM_BUFFER_BINDING_POINT));

	
	vec.push_back(std::string("#define PBR_PROBES_BUFFER_BINDINPOINT ") + std::to_string(PBR_PROBES_BUFFER_BINDINPOINT));
	vec.push_back(std::string("#define PBR_ENVIRONMENT_LIGHTS_GLOBAL_LIGHT_INDICES ") + std::to_string(PBR_ENVIRONMENT_LIGHTS_GLOBAL_LIGHT_INDICES));
	vec.push_back(std::string("#define PBR_ENVIRONMENT_LIGHTS_LIGHT_GRIDS ") + std::to_string(PBR_ENVIRONMENT_LIGHTS_LIGHT_GRIDS));
	vec.push_back(std::string("#define PBR_CLUSTERS_AABB ") + std::to_string(PBR_CLUSTERS_AABB));



	if (useConeTracing)
		vec.push_back(std::string("#define USE_CONE_TRACING"));

	return vec;
}