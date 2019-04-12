#include <nex/shader/PBRShader.hpp>
#include <glm/glm.hpp>
#include <nex/mesh/SubMesh.hpp>
#include <nex/material/Material.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nex/texture/TextureManager.hpp"

using namespace glm;
using namespace std;
using namespace nex;

PbrCommonGeometryPass::PbrCommonGeometryPass() : mProjectionMatrixSource(nullptr), mViewMatrixSource(nullptr),
                                                        mProgram(nullptr)
{
	assert(mProgram == nullptr);
}

void PbrCommonGeometryPass::init(Shader* program)
{
	mProgram = program;
	assert(mProgram != nullptr);
	// mesh material
	mAlbedoMap = mProgram->createTextureUniform("material.albedoMap", UniformType::TEXTURE2D, ALBEDO_BINDING_POINT);

	mAmbientOcclusionMap = mProgram->createTextureUniform("material.aoMap", UniformType::TEXTURE2D, AO_BINDING_POINT);
	mMetalMap = mProgram->createTextureUniform("material.metallicMap", UniformType::TEXTURE2D, METALLIC_BINDING_POINT);
	mNormalMap = mProgram->createTextureUniform("material.normalMap", UniformType::TEXTURE2D, NORMAL_BINDING_POINT);
	mRoughnessMap = mProgram->createTextureUniform("material.roughnessMap", UniformType::TEXTURE2D, ROUGHNESS_BINDING_POINT);

	mModelView = { mProgram->getUniformLocation("modelView"), UniformType::MAT4 };
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };

	mNearFarPlane = { mProgram->getUniformLocation("nearFarPlane"), UniformType::VEC2 };

	mDefaultImageSampler = TextureManager::get()->getDefaultImageSampler();
}


void PbrCommonGeometryPass::setAlbedoMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(texture, mDefaultImageSampler, mAlbedoMap.bindingSlot);
}

void PbrCommonGeometryPass::setAmbientOcclusionMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(texture, mDefaultImageSampler, mAmbientOcclusionMap.bindingSlot);
}

void PbrCommonGeometryPass::setMetalMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(texture, mDefaultImageSampler, mMetalMap.bindingSlot);
}

void PbrCommonGeometryPass::setNormalMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(texture, mDefaultImageSampler, mNormalMap.bindingSlot);
}

void PbrCommonGeometryPass::setRoughnessMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(texture, mDefaultImageSampler, mRoughnessMap.bindingSlot);
}

void PbrCommonGeometryPass::setModelViewMatrix(const glm::mat4& mat)
{
	assert(mProgram != nullptr);
	mProgram->setMat4(mModelView.location, mat);
}

void PbrCommonGeometryPass::setTransform(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
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
	mProgram->setVec2(mNearFarPlane.location, nearFarPlane);
}

void PbrCommonGeometryPass::doModelMatrixUpdate(const glm::mat4& model)
{
	assert(mViewMatrixSource != nullptr);
	assert(mProjectionMatrixSource != nullptr);
	mat4 modelView = *mViewMatrixSource * model;
	setModelViewMatrix(modelView);
	setTransform(*mProjectionMatrixSource * modelView);
}

PbrCommonLightingPass::PbrCommonLightingPass(const CascadedShadow& cascadedShadow) : mProgram(nullptr),
cascadeBufferUBO(0, CascadedShadow::CascadeData::calcCascadeDataByteSize(cascadedShadow.getCascadeData().numCascades), ShaderBuffer::UsageHint::DYNAMIC_COPY)
{

	mCascadedShadowMapSampler.setMinFilter(TextureFilter::NearestNeighbor);
	mCascadedShadowMapSampler.setMagFilter(TextureFilter::NearestNeighbor);
}

void PbrCommonLightingPass::init(Shader* program)
{
	mProgram = program;
	assert(mProgram != nullptr);

	// ibl
	mIrradianceMap = mProgram->createTextureUniform("irradianceMap", UniformType::CUBE_MAP, 5);
	mPrefilterMap = mProgram->createTextureUniform("prefilterMap", UniformType::CUBE_MAP, 6);
	mBrdfLUT = mProgram->createTextureUniform("brdfLUT", UniformType::TEXTURE2D, 7);

	// shaodw mapping
	mCascadedDepthMap = mProgram->createTextureUniform("cascadedDepthMap", UniformType::TEXTURE2D_ARRAY, 8);


	mEyeLightDirection = { mProgram->getUniformLocation("dirLight.directionEye"), UniformType::VEC3 };
	mLightColor = { mProgram->getUniformLocation("dirLight.color"), UniformType::VEC3 };
	mLightPower = { mProgram->getUniformLocation("dirLight.power"), UniformType::FLOAT };
	mAmbientLightPower = { mProgram->getUniformLocation("ambientLightPower"), UniformType::FLOAT };
	mShadowStrength = { mProgram->getUniformLocation("shadowStrength"), UniformType::FLOAT };

	mInverseView = { mProgram->getUniformLocation("inverseViewMatrix"), UniformType::MAT4 };

	mNearFarPlane = { mProgram->getUniformLocation("nearFarPlane"), UniformType::VEC2 };
}

void PbrCommonLightingPass::setBrdfLookupTexture(const Texture* brdfLUT)
{
	mProgram->setTexture(brdfLUT, &mSampler, mBrdfLUT.bindingSlot);
}

void PbrCommonLightingPass::setIrradianceMap(const CubeMap* irradianceMap)
{
	mProgram->setTexture(irradianceMap, &mSampler, mIrradianceMap.bindingSlot);
}

void PbrCommonLightingPass::setPrefilterMap(const CubeMap* prefilterMap)
{
	mProgram->setTexture(prefilterMap, &mSampler, mPrefilterMap.bindingSlot);
}

void PbrCommonLightingPass::setCascadedDepthMap(const Texture* cascadedDepthMap)
{
	mProgram->setTexture(cascadedDepthMap, &mCascadedShadowMapSampler, mCascadedDepthMap.bindingSlot);
}

void PbrCommonLightingPass::setCascadedData(const CascadedShadow::CascadeData& cascadedData, Camera* camera)
{
	//glBindBufferBase(GL_UNIFORM_BUFFER, 0, cascadeBufferUBO);
	//cascadeBufferUBO.bind();

	auto* buffer = (ShaderStorageBuffer*)&cascadeBufferUBO; // UniformBuffer ShaderStorageBuffer
	buffer->bind();
	//glNamedBufferSubData(cascadeBufferUBO, 0, sizeof(CascadedShadowGL::CascadeData), cascadedData);

	assert(cascadeBufferUBO.getSize() == cascadedData.shaderBuffer.size());

	buffer->update(cascadedData.shaderBuffer.data(), cascadedData.shaderBuffer.size(), 0);


	struct Data
	{
		mat4 inverseViewMatrix;
		mat4 lightViewProjectionMatrices[4];
		vec4 scaleFactors[4];
		vec4 cascadedSplits[4];
	};

	static auto* data = (Data*)nullptr;
	data = (Data*)cascadedData.shaderBuffer.data();
	mat4 test = glm::translate(mat4(1.0), vec3(0, 1, 0));

	//Data input;
	//input.inverseViewMatrix = inverse(camera->getView());//cascadedData.inverseViewMatrix;
	//for (int i = 0; i < 4; ++i) {
	//	input.lightViewProjectionMatrices[i] = cascadedData.lightViewProjectionMatrices[i];
	//	input.scaleFactors[i] = cascadedData.scaleFactors[i];
	//	input.cascadedSplits[i] = cascadedData.cascadedFarPlanes[i];
	//}

	//input.lightViewProjectionMatrices[3] = mat4(1.0);
	//input.lightViewProjectionMatrices[3][3][1] = 1.0f;



	//buffer->update(&input, sizeof(input), 0);



	//auto* data = (Data*)buffer->map(ShaderBuffer::Access::READ_ONLY);
	//buffer->unmap();
}

void PbrCommonLightingPass::setCascadedData(ShaderStorageBuffer* buffer)
{
	buffer->bind(0);
	//buffer->syncWithGPU();
	//uniformBuffer->map(ShaderBuffer::Access::READ_WRITE);
	//uniformBuffer->unmap();
}

void PbrCommonLightingPass::setEyeLightDirection(const glm::vec3& direction)
{
	mProgram->setVec3(mEyeLightDirection.location, direction);
}

void PbrCommonLightingPass::setLightColor(const glm::vec3& color)
{
	mProgram->setVec3(mLightColor.location, color);
}

void PbrCommonLightingPass::setLightPower(float power)
{
	mProgram->setFloat(mLightPower.location, power);
}

void PbrCommonLightingPass::setAmbientLightPower(float power)
{
	mProgram->setFloat(mAmbientLightPower.location, power);
}

void PbrCommonLightingPass::setShadowStrength(float strength)
{
	mProgram->setFloat(mShadowStrength.location, strength);
}

void PbrCommonLightingPass::setInverseViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseView.location, mat);
}

void PbrCommonLightingPass::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mProgram->setVec2(mNearFarPlane.location, nearFarPlane);
}

PbrForwardPass::PbrForwardPass(const CascadedShadow& cascadedShadow) : Pass(Shader::create(
																	"pbr/pbr_forward_vs.glsl", "pbr/pbr_forward_fs.glsl", "",
																	cascadedShadow.generateCsmDefines())),
                                                             PbrCommonLightingPass(cascadedShadow)
{
	PbrCommonGeometryPass::init(Pass::mShader.get());
	PbrCommonLightingPass::init(Pass::mShader.get());

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
	doModelMatrixUpdate(modelMatrix);
}

void PbrForwardPass::onMaterialUpdate(const Material* materialSource)
{
	const PbrMaterial* material = reinterpret_cast<const PbrMaterial*>(materialSource);

	if (material == nullptr)
		return;

	setAlbedoMap(material->getAlbedoMap());
	setAmbientOcclusionMap(material->getAoMap());
	setMetalMap(material->getMetallicMap());
	setNormalMap(material->getNormalMap());
	setRoughnessMap(material->getRoughnessMap());
}

PbrDeferredLightingPass::PbrDeferredLightingPass(const CascadedShadow& cascadedShadow) : PbrCommonLightingPass(cascadedShadow)
{

	Pass::mShader = Shader::create(
		"pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs_optimized.glsl", "", cascadedShadow.generateCsmDefines());

	PbrCommonLightingPass::init(Pass::mShader.get());

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


PbrDeferredGeometryPass::PbrDeferredGeometryPass()
{
	Pass::mShader = Shader::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl", "pbr/pbr_deferred_geometry_pass_fs.glsl");

	init(Pass::mShader.get());
}

void PbrDeferredGeometryPass::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	doModelMatrixUpdate(modelMatrix);
}

void PbrDeferredGeometryPass::onMaterialUpdate(const Material* materialSource)
{
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