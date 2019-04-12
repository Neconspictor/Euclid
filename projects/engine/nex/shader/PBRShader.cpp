#include <nex/shader/PBRShader.hpp>
#include <glm/glm.hpp>
#include <nex/mesh/SubMesh.hpp>
#include <nex/material/Material.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nex/texture/TextureManager.hpp"

using namespace glm;
using namespace std;
using namespace nex;

PbrCommonGeometryShader::PbrCommonGeometryShader() : mProjectionMatrixSource(nullptr), mViewMatrixSource(nullptr),
                                                        mProgram(nullptr)
{
	assert(mProgram == nullptr);
}

void PbrCommonGeometryShader::init(ShaderProgram* program)
{
	mProgram = program;
	assert(mProgram != nullptr);
	// mesh material
	mAlbedoMap = mProgram->createTextureUniform("material.albedoMap", UniformType::TEXTURE2D, 0);

	mAmbientOcclusionMap = mProgram->createTextureUniform("material.aoMap", UniformType::TEXTURE2D, 1);
	mMetalMap = mProgram->createTextureUniform("material.metallicMap", UniformType::TEXTURE2D, 2);
	mNormalMap = mProgram->createTextureUniform("material.normalMap", UniformType::TEXTURE2D, 3);
	mRoughnessMap = mProgram->createTextureUniform("material.roughnessMap", UniformType::TEXTURE2D, 4);

	mModelView = { mProgram->getUniformLocation("modelView"), UniformType::MAT4 };
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };

	mNearFarPlane = { mProgram->getUniformLocation("nearFarPlane"), UniformType::VEC2 };

	mDefaultImageSampler = TextureManager::get()->getDefaultImageSampler();
}


void PbrCommonGeometryShader::setAlbedoMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(texture, mDefaultImageSampler, mAlbedoMap.bindingSlot);
}

void PbrCommonGeometryShader::setAmbientOcclusionMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(texture, mDefaultImageSampler, mAmbientOcclusionMap.bindingSlot);
}

void PbrCommonGeometryShader::setMetalMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(texture, mDefaultImageSampler, mMetalMap.bindingSlot);
}

void PbrCommonGeometryShader::setNormalMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(texture, mDefaultImageSampler, mNormalMap.bindingSlot);
}

void PbrCommonGeometryShader::setRoughnessMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(texture, mDefaultImageSampler, mRoughnessMap.bindingSlot);
}

void PbrCommonGeometryShader::setModelViewMatrix(const glm::mat4& mat)
{
	assert(mProgram != nullptr);
	mProgram->setMat4(mModelView.location, mat);
}

void PbrCommonGeometryShader::setTransform(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void PbrCommonGeometryShader::setProjection(const glm::mat4& mat)
{
	mProjectionMatrixSource = &mat;
}

void PbrCommonGeometryShader::setView(const glm::mat4& mat)
{
	mViewMatrixSource = &mat;
}

void PbrCommonGeometryShader::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mProgram->setVec2(mNearFarPlane.location, nearFarPlane);
}

void PbrCommonGeometryShader::doModelMatrixUpdate(const glm::mat4& model)
{
	assert(mViewMatrixSource != nullptr);
	assert(mProjectionMatrixSource != nullptr);
	mat4 modelView = *mViewMatrixSource * model;
	setModelViewMatrix(modelView);
	setTransform(*mProjectionMatrixSource * modelView);
}

PbrCommonLightingShader::PbrCommonLightingShader(const CascadedShadow& cascadedShadow) : mProgram(nullptr),
cascadeBufferUBO(0, CascadedShadow::CascadeData::calcCascadeDataByteSize(cascadedShadow.getCascadeData().numCascades), ShaderBuffer::UsageHint::DYNAMIC_COPY)
{

	mCascadedShadowMapSampler.setMinFilter(TextureFilter::NearestNeighbor);
	mCascadedShadowMapSampler.setMagFilter(TextureFilter::NearestNeighbor);
}

void PbrCommonLightingShader::init(ShaderProgram* program)
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

void PbrCommonLightingShader::setBrdfLookupTexture(const Texture* brdfLUT)
{
	mProgram->setTexture(brdfLUT, &mSampler, mBrdfLUT.bindingSlot);
}

void PbrCommonLightingShader::setIrradianceMap(const CubeMap* irradianceMap)
{
	mProgram->setTexture(irradianceMap, &mSampler, mIrradianceMap.bindingSlot);
}

void PbrCommonLightingShader::setPrefilterMap(const CubeMap* prefilterMap)
{
	mProgram->setTexture(prefilterMap, &mSampler, mPrefilterMap.bindingSlot);
}

void PbrCommonLightingShader::setCascadedDepthMap(const Texture* cascadedDepthMap)
{
	mProgram->setTexture(cascadedDepthMap, &mCascadedShadowMapSampler, mCascadedDepthMap.bindingSlot);
}

void PbrCommonLightingShader::setCascadedData(const CascadedShadow::CascadeData& cascadedData, Camera* camera)
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

void PbrCommonLightingShader::setCascadedData(ShaderStorageBuffer* buffer)
{
	buffer->bind(0);
	//buffer->syncWithGPU();
	//uniformBuffer->map(ShaderBuffer::Access::READ_WRITE);
	//uniformBuffer->unmap();
}

void PbrCommonLightingShader::setEyeLightDirection(const glm::vec3& direction)
{
	mProgram->setVec3(mEyeLightDirection.location, direction);
}

void PbrCommonLightingShader::setLightColor(const glm::vec3& color)
{
	mProgram->setVec3(mLightColor.location, color);
}

void PbrCommonLightingShader::setLightPower(float power)
{
	mProgram->setFloat(mLightPower.location, power);
}

void PbrCommonLightingShader::setAmbientLightPower(float power)
{
	mProgram->setFloat(mAmbientLightPower.location, power);
}

void PbrCommonLightingShader::setShadowStrength(float strength)
{
	mProgram->setFloat(mShadowStrength.location, strength);
}

void PbrCommonLightingShader::setInverseViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseView.location, mat);
}

void PbrCommonLightingShader::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mProgram->setVec2(mNearFarPlane.location, nearFarPlane);
}

PbrForwardShader::PbrForwardShader(const CascadedShadow& cascadedShadow) : Shader(ShaderProgram::create(
																	"pbr/pbr_forward_vs.glsl", "pbr/pbr_forward_fs.glsl", "",
																	cascadedShadow.generateCsmDefines())),
                                                             PbrCommonLightingShader(cascadedShadow)
{
	PbrCommonGeometryShader::init(Shader::mProgram.get());
	PbrCommonLightingShader::init(Shader::mProgram.get());

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

void PbrForwardShader::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	doModelMatrixUpdate(modelMatrix);
}

void PbrForwardShader::onMaterialUpdate(const Material* materialSource)
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

PbrDeferredLightingShader::PbrDeferredLightingShader(const CascadedShadow& cascadedShadow) : PbrCommonLightingShader(cascadedShadow)
{

	Shader::mProgram = ShaderProgram::create(
		"pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs_optimized.glsl", "", cascadedShadow.generateCsmDefines());

	PbrCommonLightingShader::init(Shader::mProgram.get());

	mTransform = { Shader::mProgram->getUniformLocation("transform"), UniformType::MAT4 };


	mAlbedoMap = Shader::mProgram->createTextureUniform("gBuffer.albedoMap", UniformType::TEXTURE2D, 0);
	mAoMetalRoughnessMap = Shader::mProgram->createTextureUniform("gBuffer.aoMetalRoughnessMap", UniformType::TEXTURE2D, 1);
	mNormalEyeMap = Shader::mProgram->createTextureUniform("gBuffer.normalEyeMap", UniformType::TEXTURE2D, 2);
	mNormalizedViewSpaceZMap = Shader::mProgram->createTextureUniform("gBuffer.normalizedViewSpaceZMap", UniformType::TEXTURE2D, 3);

	mInverseProjFromGPass = { Shader::mProgram->getUniformLocation("inverseProjMatrix_GPass"), UniformType::MAT4 };

	auto state = Shader::mSampler.getState();
	state.wrapR = state.wrapS = state.wrapT = TextureUVTechnique::ClampToEdge;
	state.minFilter = state.magFilter = TextureFilter::NearestNeighbor;
	Shader::mSampler.setState(state);
}

void PbrDeferredLightingShader::setMVP(const glm::mat4& trafo)
{
	Shader::mProgram->setMat4(mTransform.location, trafo);
}

void PbrDeferredLightingShader::setAlbedoMap(const Texture* texture)
{
	Shader::mProgram->setTexture(texture, &(Shader::mSampler), mAlbedoMap.bindingSlot);
}

void PbrDeferredLightingShader::setAoMetalRoughnessMap(const Texture* texture)
{
	Shader::mProgram->setTexture(texture, &(Shader::mSampler), mAoMetalRoughnessMap.bindingSlot);
}

void PbrDeferredLightingShader::setNormalEyeMap(const Texture* texture)
{
	Shader::mProgram->setTexture(texture, &(Shader::mSampler), mNormalEyeMap.bindingSlot);
}

void PbrDeferredLightingShader::setNormalizedViewSpaceZMap(const Texture* texture)
{
	Shader::mProgram->setTexture(texture, &(Shader::mSampler), mNormalizedViewSpaceZMap.bindingSlot);
}

void PbrDeferredLightingShader::setInverseProjMatrixFromGPass(const glm::mat4& mat)
{
	Shader::mProgram->setMat4(mInverseProjFromGPass.location, mat);
}

void PbrDeferredLightingShader::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection) * (*data.view) * (*data.model));
}


PbrDeferredGeometryShader::PbrDeferredGeometryShader()
{
	Shader::mProgram = ShaderProgram::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl", "pbr/pbr_deferred_geometry_pass_fs.glsl");

	init(Shader::mProgram.get());
}

void PbrDeferredGeometryShader::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	doModelMatrixUpdate(modelMatrix);
}

void PbrDeferredGeometryShader::onMaterialUpdate(const Material* materialSource)
{
}

PbrConvolutionShader::PbrConvolutionShader()
{
	mProgram = ShaderProgram::create(
		"pbr/pbr_convolution_vs.glsl", "pbr/pbr_convolution_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };

	mEnvironmentMap = mProgram->createTextureUniform("environmentMap", UniformType::CUBE_MAP, 0);
}

void PbrConvolutionShader::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PbrConvolutionShader::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void PbrConvolutionShader::setEnvironmentMap(const CubeMap * cubeMap)
{
	mProgram->setTexture(cubeMap, &mSampler, mEnvironmentMap.bindingSlot);
	mProgram->setBinding(mEnvironmentMap.location, mEnvironmentMap.bindingSlot);
}

PbrPrefilterShader::PbrPrefilterShader()
{
	mProgram = ShaderProgram::create(
		"pbr/pbr_prefilter_cubemap_vs.glsl", "pbr/pbr_prefilter_cubemap_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mRoughness = { mProgram->getUniformLocation("roughness"), UniformType::FLOAT };

	//mEnvironmentMap = mProgram->createTextureUniform("environmentMap", UniformType::CUBE_MAP, 0);
	mEnvironmentMap = { mProgram->getUniformLocation("environmentMap"), UniformType::CUBE_MAP, 0 };
}

void PbrPrefilterShader::setMapToPrefilter(CubeMap * cubeMap)
{
	mProgram->setTexture(cubeMap, &mSampler, mEnvironmentMap.bindingSlot);
	mProgram->setBinding(mEnvironmentMap.location, mEnvironmentMap.bindingSlot);
}

void PbrPrefilterShader::setRoughness(float roughness)
{
	mProgram->setFloat(mRoughness.location, roughness);
}

void PbrPrefilterShader::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PbrPrefilterShader::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

PbrBrdfPrecomputeShader::PbrBrdfPrecomputeShader()
{
	mProgram = ShaderProgram::create(
		"pbr/pbr_brdf_precompute_vs.glsl", "pbr/pbr_brdf_precompute_fs.glsl");

	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
}

void PbrBrdfPrecomputeShader::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void PbrBrdfPrecomputeShader::onTransformUpdate(const TransformData& data)
{
	setMVP(*data.projection * (*data.view) * (*data.model));
}