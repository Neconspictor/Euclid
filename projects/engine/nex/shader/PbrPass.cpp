#include <nex/shader/PbrPass.hpp>
#include <glm/glm.hpp>
#include <nex/mesh/SubMesh.hpp>
#include <nex/material/Material.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nex/texture/TextureManager.hpp"
#include "nex/pbr/PbrProbe.hpp"

using namespace glm;
using namespace std;
using namespace nex;

PbrCommonGeometryPass::PbrCommonGeometryPass(Shader* shader) : PbrBaseCommon(shader),
mProjectionMatrixSource(nullptr), mViewMatrixSource(nullptr), mDefaultImageSampler(nullptr)
{
	assert(mShader != nullptr);

	// mesh material
	mAlbedoMap = mShader->createTextureUniform("material.albedoMap", UniformType::TEXTURE2D, ALBEDO_BINDING_POINT);

	mAmbientOcclusionMap = mShader->createTextureUniform("material.aoMap", UniformType::TEXTURE2D, AO_BINDING_POINT);
	mMetalMap = mShader->createTextureUniform("material.metallicMap", UniformType::TEXTURE2D, METALLIC_BINDING_POINT);
	mNormalMap = mShader->createTextureUniform("material.normalMap", UniformType::TEXTURE2D, NORMAL_BINDING_POINT);
	mRoughnessMap = mShader->createTextureUniform("material.roughnessMap", UniformType::TEXTURE2D, ROUGHNESS_BINDING_POINT);

	mModelView = { mShader->getUniformLocation("modelView"), UniformType::MAT4 };
	mTransform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };

	mNearFarPlane = { mShader->getUniformLocation("nearFarPlane"), UniformType::VEC2 };

	mDefaultImageSampler = TextureManager::get()->getDefaultImageSampler();
}


void PbrCommonGeometryPass::setModelViewMatrix(const glm::mat4& mat)
{
	assert(mShader != nullptr);
	mShader->setMat4(mModelView.location, mat);
}

void PbrCommonGeometryPass::setTransform(const glm::mat4& mat)
{
	mShader->setMat4(mTransform.location, mat);
}

void PbrCommonGeometryPass::setProjection(const glm::mat4& mat)
{
	mProjectionMatrixSource = &mat;
}

void PbrCommonGeometryPass::setView(const glm::mat4& mat)
{
	mViewMatrixSource = &mat;
}

void PbrCommonGeometryPass::setNearFarPlane(const glm::vec2& nearFarPlane)
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

void PbrCommonGeometryPass::doModelMatrixUpdate(const glm::mat4& model)
{
	assert(mViewMatrixSource != nullptr);
	assert(mProjectionMatrixSource != nullptr);
	mat4 modelView = *mViewMatrixSource * model;
	setModelViewMatrix(modelView);
	setTransform(*mProjectionMatrixSource * modelView);
}

void PbrCommonGeometryPass::updateConstants(Camera* camera)
{
	mDefaultImageSampler->bind(ALBEDO_BINDING_POINT);
	mDefaultImageSampler->bind(AO_BINDING_POINT);
	mDefaultImageSampler->bind(METALLIC_BINDING_POINT);
	mDefaultImageSampler->bind(NORMAL_BINDING_POINT);
	mDefaultImageSampler->bind(ROUGHNESS_BINDING_POINT);

	setView(camera->getView());
	setProjection(camera->getPerspProjection());
	setNearFarPlane(camera->getNearFarPlaneViewSpace(Perspective));
}

void PbrCommonLightingPass::setBrdfLookupTexture(const Texture* brdfLUT)
{
	mShader->setTexture(brdfLUT, &mSampler, mBrdfLUT.bindingSlot);
}

void PbrCommonLightingPass::setIrradianceMap(const CubeMap* irradianceMap)
{
	mShader->setTexture(irradianceMap, &mSampler, mIrradianceMap.bindingSlot);
}

void PbrCommonLightingPass::setPrefilterMap(const CubeMap* prefilterMap)
{
	mShader->setTexture(prefilterMap, &mSampler, mPrefilterMap.bindingSlot);
}

void PbrCommonLightingPass::setCascadedDepthMap(const Texture* cascadedDepthMap)
{
	mShader->setTexture(cascadedDepthMap, &mCascadedShadowMapSampler, mCascadedDepthMap.bindingSlot);
}

void PbrCommonLightingPass::setCascadedData(const CascadedShadow::CascadeData& cascadedData)
{
	auto* buffer = (ShaderStorageBuffer*)&cascadeBufferUBO; // UniformBuffer ShaderStorageBuffer
	buffer->bind();

	assert(cascadeBufferUBO.getSize() == cascadedData.shaderBuffer.size());

	buffer->update(cascadedData.shaderBuffer.data(), cascadedData.shaderBuffer.size(), 0);
}

void PbrCommonLightingPass::setEyeLightDirection(const glm::vec3& direction)
{
	mShader->setVec3(mEyeLightDirection.location, direction);
}

void PbrCommonLightingPass::setLightColor(const glm::vec3& color)
{
	mShader->setVec3(mLightColor.location, color);
}

void PbrCommonLightingPass::setLightPower(float power)
{
	mShader->setFloat(mLightPower.location, power);
}

void PbrCommonLightingPass::setAmbientLightPower(float power)
{
	mShader->setFloat(mAmbientLightPower.location, power);
}

void PbrCommonLightingPass::setShadowStrength(float strength)
{
	mShader->setFloat(mShadowStrength.location, strength);
}

void PbrCommonLightingPass::setInverseViewMatrix(const glm::mat4& mat)
{
	mShader->setMat4(mInverseView.location, mat);
}

void PbrCommonLightingPass::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mShader->setVec2(mNearFarPlane.location, nearFarPlane);
}

void PbrCommonLightingPass::setProbe(PbrProbe* probe)
{
	mProbe = probe;
}

nex::PbrCommonLightingPass::PbrCommonLightingPass(Shader * shader, CascadedShadow* cascadedShadow) :
PbrBaseCommon(shader),
cascadeBufferUBO(0, CascadedShadow::CascadeData::calcCascadeDataByteSize(cascadedShadow->getCascadeData().numCascades), ShaderBuffer::UsageHint::DYNAMIC_COPY),
mCascadeShadow(cascadedShadow)
{
	assert(mShader != nullptr);
	assert(mCascadeShadow != nullptr);

	mCascadedShadowMapSampler.setMinFilter(TextureFilter::NearestNeighbor);
	mCascadedShadowMapSampler.setMagFilter(TextureFilter::NearestNeighbor);

	// ibl
	mIrradianceMap = mShader->createTextureUniform("irradianceMap", UniformType::CUBE_MAP, 5);
	mPrefilterMap = mShader->createTextureUniform("prefilterMap", UniformType::CUBE_MAP, 6);
	mBrdfLUT = mShader->createTextureUniform("brdfLUT", UniformType::TEXTURE2D, 7);

	// shaodw mapping
	mCascadedDepthMap = mShader->createTextureUniform("cascadedDepthMap", UniformType::TEXTURE2D_ARRAY, 8);


	mEyeLightDirection = { mShader->getUniformLocation("dirLight.directionEye"), UniformType::VEC3 };
	mLightColor = { mShader->getUniformLocation("dirLight.color"), UniformType::VEC3 };
	mLightPower = { mShader->getUniformLocation("dirLight.power"), UniformType::FLOAT };
	mAmbientLightPower = { mShader->getUniformLocation("ambientLightPower"), UniformType::FLOAT };
	mShadowStrength = { mShader->getUniformLocation("shadowStrength"), UniformType::FLOAT };

	mInverseView = { mShader->getUniformLocation("inverseViewMatrix"), UniformType::MAT4 };

	mNearFarPlane = { mShader->getUniformLocation("nearFarPlane"), UniformType::VEC2 };
}

void PbrCommonLightingPass::setAmbientLight(AmbientLight* light)
{
	mAmbientLight = light;
}

void PbrCommonLightingPass::setCascadedShadow(CascadedShadow* shadow)
{
	mCascadeShadow = shadow;
}

void PbrCommonLightingPass::setDirLight(DirectionalLight* light)
{
	mLight = light;
}

void PbrCommonLightingPass::updateConstants(Camera* camera)
{
	assert(mLight != nullptr);
	assert(mAmbientLight != nullptr);
	assert(mCascadeShadow != nullptr);
	assert(mProbe != nullptr);
	assert(camera != nullptr);
	


	setBrdfLookupTexture(mProbe->getBrdfLookupTexture());
	setIrradianceMap(mProbe->getConvolutedEnvironmentMap());
	setPrefilterMap(mProbe->getPrefilteredEnvironmentMap());

	setAmbientLightPower(mAmbientLight->getPower());
	setLightColor(mLight->getColor());
	setLightPower(mLight->getLightPower());

	vec4 lightEyeDirection = camera->getView() * vec4(mLight->getDirection(), 0);
	setEyeLightDirection(vec3(lightEyeDirection));

	setInverseViewMatrix(inverse(camera->getView()));

	setNearFarPlane(camera->getNearFarPlaneViewSpace(Perspective));
	setShadowStrength(mCascadeShadow->getShadowStrength());
	setCascadedData(mCascadeShadow->getCascadeData());

	auto* buffer = mCascadeShadow->getCascadeBuffer();
	buffer->bind(0);
	
	setCascadedDepthMap(mCascadeShadow->getDepthTextureArray());
}

PbrForwardPass::PbrForwardPass(CascadedShadow* cascadedShadow) : 
	Pass(Shader::create("pbr/pbr_forward_vs.glsl", "pbr/pbr_forward_fs.glsl", "", cascadedShadow->generateCsmDefines())),
	mGeometryPass(mShader.get()),
    mLightingPass(mShader.get(), cascadedShadow)
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

void PbrForwardPass::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	mGeometryPass.doModelMatrixUpdate(modelMatrix);
}

void PbrForwardPass::onMaterialUpdate(const Material* materialSource)
{
}

void PbrForwardPass::updateConstants(Camera* camera)
{
	bind();
	mGeometryPass.updateConstants(camera);
	mLightingPass.updateConstants(camera);
}

void PbrForwardPass::setProbe(PbrProbe* probe)
{
	mLightingPass.setProbe(probe);
}

void PbrForwardPass::setAmbientLight(AmbientLight* light)
{
	mLightingPass.setAmbientLight(light);
}

void PbrForwardPass::setDirLight(DirectionalLight* light)
{
	mLightingPass.setDirLight(light);
}

PbrDeferredLightingPass::PbrDeferredLightingPass(CascadedShadow* cascadedShadow) : 
	TransformPass(Shader::create("pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs_optimized.glsl", "", cascadedShadow->generateCsmDefines())),
	mLightingPass(mShader.get(), cascadedShadow)
{
	mTransform = { Pass::mShader->getUniformLocation("transform"), UniformType::MAT4 };

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

void PbrDeferredLightingPass::setMVP(const glm::mat4& trafo)
{
	Pass::mShader->setMat4(mTransform.location, trafo);
}

void PbrDeferredLightingPass::setAlbedoMap(const Texture* texture)
{
	Pass::mShader->setTexture(texture, &(Pass::mSampler), mAlbedoMap.bindingSlot);
}

void PbrDeferredLightingPass::setAoMetalRoughnessMap(const Texture* texture)
{
	Pass::mShader->setTexture(texture, &(Pass::mSampler), mAoMetalRoughnessMap.bindingSlot);
}

void PbrDeferredLightingPass::setNormalEyeMap(const Texture* texture)
{
	Pass::mShader->setTexture(texture, &(Pass::mSampler), mNormalEyeMap.bindingSlot);
}

void PbrDeferredLightingPass::setNormalizedViewSpaceZMap(const Texture* texture)
{
	Pass::mShader->setTexture(texture, &(Pass::mSampler), mNormalizedViewSpaceZMap.bindingSlot);
}

void PbrDeferredLightingPass::setInverseProjMatrixFromGPass(const glm::mat4& mat)
{
	Pass::mShader->setMat4(mInverseProjFromGPass.location, mat);
}

void PbrDeferredLightingPass::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection) * (*data.view) * (*data.model));
}

void PbrDeferredLightingPass::updateConstants(Camera* camera)
{
	bind();
	mLightingPass.updateConstants(camera);
	setInverseProjMatrixFromGPass(inverse(camera->getPerspProjection()));
}

void PbrDeferredLightingPass::setProbe(PbrProbe* probe)
{
	mLightingPass.setProbe(probe);
}

void PbrDeferredLightingPass::setAmbientLight(AmbientLight* light)
{
	mLightingPass.setAmbientLight(light);
}

void PbrDeferredLightingPass::setDirLight(DirectionalLight* light)
{
	mLightingPass.setDirLight(light);
}


PbrDeferredGeometryPass::PbrDeferredGeometryPass() : 
	Pass(Shader::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl", "pbr/pbr_deferred_geometry_pass_fs.glsl")),
	mGeometryPass(mShader.get())
{
}

void PbrDeferredGeometryPass::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	mGeometryPass.doModelMatrixUpdate(modelMatrix);
}

void PbrDeferredGeometryPass::onMaterialUpdate(const Material* materialSource)
{
}

void PbrDeferredGeometryPass::updateConstants(Camera* camera)
{
	bind();
	mGeometryPass.updateConstants(camera);
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

	mTransform = { mShader->getUniformLocation("transform"), UniformType::MAT4 };
}

void PbrBrdfPrecomputePass::setMVP(const glm::mat4& mat)
{
	mShader->setMat4(mTransform.location, mat);
}

void PbrBrdfPrecomputePass::onTransformUpdate(const TransformData& data)
{
	setMVP(*data.projection * (*data.view) * (*data.model));
}