#include <nex/pbr/PbrPass.hpp>
#include <glm/glm.hpp>
#include <nex/mesh/Mesh.hpp>
#include <nex/material/Material.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nex/texture/TextureManager.hpp"
#include "nex/pbr/PbrProbe.hpp"
#include <nex/pbr/GlobalIllumination.hpp>

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

void PbrGeometryData::updateConstants(Camera* camera)
{
	/*mDefaultImageSampler->bind(ALBEDO_BINDING_POINT);
	mDefaultImageSampler->bind(AO_BINDING_POINT);
	mDefaultImageSampler->bind(METALLIC_BINDING_POINT);
	mDefaultImageSampler->bind(NORMAL_BINDING_POINT);
	mDefaultImageSampler->bind(ROUGHNESS_BINDING_POINT);*/

	setNearFarPlane(camera->getNearFarPlaneViewSpace());
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
	CascadedShadow* cascadedShadow, unsigned csmCascadeBindingPoint, unsigned pbrProbesBufferBindingPoint) :
	PbrBaseCommon(shader),
	mPbrProbesDataBufferBindingPoint(pbrProbesBufferBindingPoint),
	mGlobalIllumination(globalIllumination),
	mCsmCascadeBindingPoint(csmCascadeBindingPoint),
	
	//cascadeBufferUBO(
	//	CSM_CASCADE_BUFFER_BINDING_POINT, 
	//	CascadedShadow::CascadeData::calcCascadeDataByteSize(cascadedShadow->getCascadeData().numCascades),
	//	ShaderBuffer::UsageHint::DYNAMIC_COPY), mProbe(nullptr),
	mCascadeShadow(cascadedShadow)
{
	assert(mShader != nullptr);
	assert(mCascadeShadow != nullptr);

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

void PbrLightingData::setDirLight(DirectionalLight* light)
{
	mLight = light;
}

void PbrLightingData::updateConstants(Camera* camera)
{
	assert(mLight != nullptr);
	assert(mGlobalIllumination != nullptr);
	assert(mCascadeShadow != nullptr);
	assert(camera != nullptr);


	auto* probe = mGlobalIllumination->getActiveProbe();

	setArrayIndex((float)probe->getArrayIndex());
	setBrdfLookupTexture(probe->getBrdfLookupTexture());

	setIrradianceMaps(mGlobalIllumination->getIrradianceMaps());
	setPrefilteredMaps(mGlobalIllumination->getPrefilteredMaps());

	setAmbientLightPower(mGlobalIllumination->getAmbientPower());
	setLightColor(mLight->getColor());
	setLightPower(mLight->getLightPower());

	vec4 lightEyeDirection = camera->getView() * vec4(mLight->getDirection(), 0);
	setEyeLightDirection(vec3(lightEyeDirection));

	setInverseViewMatrix(inverse(camera->getView()));

	setNearFarPlane(camera->getNearFarPlaneViewSpace());
	setShadowStrength(mCascadeShadow->getShadowStrength());
	//setCascadedData(mCascadeShadow->getCascadeData());

	auto* buffer = mCascadeShadow->getCascadeBuffer();
	buffer->bind(mCsmCascadeBindingPoint);
	setCascadedDepthMap(mCascadeShadow->getDepthTextureArray());


	auto* probesBuffer = mGlobalIllumination->getProbesShaderBuffer();
	probesBuffer->bind(mPbrProbesDataBufferBindingPoint);
}

PbrForwardPass::PbrForwardPass(GlobalIllumination* globalIllumination, CascadedShadow* cascadedShadow) :
	PbrGeometryPass(Shader::create("pbr/pbr_forward_vs.glsl", "pbr/pbr_forward_fs.glsl", nullptr, nullptr, nullptr, generateDefines(cascadedShadow)),
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

void PbrForwardPass::updateConstants(Camera* camera)
{
	bind();
	setViewProjectionMatrices(camera->getProjectionMatrix(), camera->getView(), camera->getPrevView());

	mGeometryData.updateConstants(camera);
	mLightingPass.updateConstants(camera);
}

void PbrForwardPass::setDirLight(DirectionalLight* light)
{
	mLightingPass.setDirLight(light);
}

std::vector<std::string> PbrForwardPass::generateDefines(CascadedShadow* cascadedShadow)
{
	auto vec = cascadedShadow->generateCsmDefines();

	//CSM_CASCADE_BUFFER_BINDING_POINT

	// csm CascadeBuffer and TransformBuffer both use binding point 0 per default. Resolve this conflict by using binding point 1 
	// for the TransformBuffer
	vec.push_back(std::string("#define CSM_CASCADE_BUFFER_BINDING_POINT ") + std::to_string(CASCADE_BUFFER_BINDINGPOINT));
	vec.push_back(std::string("#define PBR_COMMON_GEOMETRY_TRANSFORM_BUFFER_BINDING_POINT ") + std::to_string(TRANSFORM_BUFFER_BINDINGPOINT));

	// probes buffer must use another binding point, too.
	vec.push_back(std::string("#define PBR_PROBES_BUFFER_BINDING_POINT ") + std::to_string(PBR_PROBES_BUFFER_BINDINPOINT));

	return vec;
}

PbrDeferredLightingPass::PbrDeferredLightingPass(GlobalIllumination* globalIllumination, CascadedShadow* cascadedShadow) :
	Pass(Shader::create("pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs.glsl", 
						nullptr, nullptr, nullptr, generateDefines(cascadedShadow))),
	mLightingPass(mShader.get(), globalIllumination, cascadedShadow, CASCADE_BUFFER_BINDINGPOINT, PBR_PROBES_BUFFER_BINDINPOINT)
{
	mAlbedoMap = Pass::mShader->createTextureUniform("gBuffer.albedoMap", UniformType::TEXTURE2D, 0);
	mAoMetalRoughnessMap = Pass::mShader->createTextureUniform("gBuffer.aoMetalRoughnessMap", UniformType::TEXTURE2D, 1);
	mNormalEyeMap = Pass::mShader->createTextureUniform("gBuffer.normalEyeMap", UniformType::TEXTURE2D, 2);
	mNormalizedViewSpaceZMap = Pass::mShader->createTextureUniform("gBuffer.normalizedViewSpaceZMap", UniformType::TEXTURE2D, 3);

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

void PbrDeferredLightingPass::setInverseProjMatrixFromGPass(const glm::mat4& mat)
{
	Pass::mShader->setMat4(mInverseProjFromGPass.location, mat);
}

void PbrDeferredLightingPass::updateConstants(Camera* camera)
{
	bind();
	mLightingPass.updateConstants(camera);
	setInverseProjMatrixFromGPass(inverse(camera->getProjectionMatrix()));
}

void PbrDeferredLightingPass::setDirLight(DirectionalLight* light)
{
	mLightingPass.setDirLight(light);
}

std::vector<std::string> nex::PbrDeferredLightingPass::generateDefines(CascadedShadow * cascadedShadow)
{
	auto vec = cascadedShadow->generateCsmDefines();

	vec.push_back(std::string("#define CSM_CASCADE_BUFFER_BINDING_POINT ") + std::to_string(CASCADE_BUFFER_BINDINGPOINT));
	vec.push_back(std::string("#define PBR_PROBES_BUFFER_BINDING_POINT ") + std::to_string(PBR_PROBES_BUFFER_BINDINPOINT));

	return vec;
}


PbrDeferredGeometryPass::PbrDeferredGeometryPass() :
	PbrGeometryPass(Shader::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl", "pbr/pbr_deferred_geometry_pass_fs.glsl"))
{
}

void PbrDeferredGeometryPass::updateConstants(Camera* camera)
{
	bind();
	setViewProjectionMatrices(camera->getProjectionMatrix(), camera->getView(), camera->getPrevView());
	mGeometryData.updateConstants(camera);
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
	mShader->setBinding(mEnvironmentMap.location, mEnvironmentMap.bindingSlot);
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
		"pbr/pbr_brdf_precompute_vs.glsl", "pbr/pbr_brdf_precompute_fs.glsl");
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
