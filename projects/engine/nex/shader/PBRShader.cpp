#include <nex/shader/PBRShader.hpp>
#include <glm/glm.hpp>
#include <nex/mesh/SubMesh.hpp>
#include <nex/material/Material.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;
using namespace nex;

pbr::CommonGeometryMaterial::CommonGeometryMaterial() : mProjectionMatrixSource(nullptr), mViewMatrixSource(nullptr),
                                                        mProgram(nullptr)
{
	assert(mProgram == nullptr);
}

void pbr::CommonGeometryMaterial::init(ShaderProgram* program)
{
	mProgram = program;
	assert(mProgram != nullptr);
	// mesh material
	mAlbedoMap = { mProgram->getUniformLocation("material.albedoMap"), UniformType::TEXTURE2D, 0 };
	mAmbientOcclusionMap = { mProgram->getUniformLocation("material.aoMap"), UniformType::TEXTURE2D, 1 };
	mMetalMap = { mProgram->getUniformLocation("material.metallicMap"), UniformType::TEXTURE2D, 2 };
	mNormalMap = { mProgram->getUniformLocation("material.normalMap"), UniformType::TEXTURE2D, 3 };
	mRoughnessMap = { mProgram->getUniformLocation("material.roughnessMap"), UniformType::TEXTURE2D, 4 };
	mModelView = { mProgram->getUniformLocation("modelView"), UniformType::MAT4 };
	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };

	mNearFarPlane = { mProgram->getUniformLocation("nearFarPlane"), UniformType::VEC2 };
}


void pbr::CommonGeometryMaterial::setAlbedoMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(mAlbedoMap.location, texture, mAlbedoMap.bindingSlot);
}

void pbr::CommonGeometryMaterial::setAmbientOcclusionMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(mAmbientOcclusionMap.location, texture, mAmbientOcclusionMap.bindingSlot);
}

void pbr::CommonGeometryMaterial::setMetalMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(mMetalMap.location, texture, mMetalMap.bindingSlot);
}

void pbr::CommonGeometryMaterial::setNormalMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(mNormalMap.location, texture, mNormalMap.bindingSlot);
}

void pbr::CommonGeometryMaterial::setRoughnessMap(const Texture* texture)
{
	assert(mProgram != nullptr);
	mProgram->setTexture(mRoughnessMap.location, texture, mRoughnessMap.bindingSlot);
}

void pbr::CommonGeometryMaterial::setModelViewMatrix(const glm::mat4& mat)
{
	assert(mProgram != nullptr);
	mProgram->setMat4(mModelView.location, mat);
}

void pbr::CommonGeometryMaterial::setTransform(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void pbr::CommonGeometryMaterial::setProjection(const glm::mat4& mat)
{
	mProjectionMatrixSource = &mat;
}

void pbr::CommonGeometryMaterial::setView(const glm::mat4& mat)
{
	mViewMatrixSource = &mat;
}

void pbr::CommonGeometryMaterial::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mProgram->setVec2(mNearFarPlane.location, nearFarPlane);
}

void pbr::CommonGeometryMaterial::doModelMatrixUpdate(const glm::mat4& model)
{
	assert(mViewMatrixSource != nullptr);
	assert(mProjectionMatrixSource != nullptr);
	mat4 modelView = *mViewMatrixSource * model;
	setModelViewMatrix(modelView);
	setTransform(*mProjectionMatrixSource * modelView);
}

pbr::CommonLightingMaterial::CommonLightingMaterial(const CascadedShadow& cascadedShadow) : mProgram(nullptr),
cascadeBufferUBO(0, CascadedShadow::CascadeData::calcCascadeDataByteSize(cascadedShadow.getCascadeData().numCascades), ShaderBuffer::UsageHint::DYNAMIC_COPY)
{
}

void pbr::CommonLightingMaterial::init(ShaderProgram* program)
{
	mProgram = program;
	assert(mProgram != nullptr);

	// ibl
	mIrradianceMap = { mProgram->getUniformLocation("irradianceMap"), UniformType::CUBE_MAP, 5 };
	mPrefilterMap = { mProgram->getUniformLocation("prefilterMap"), UniformType::CUBE_MAP, 6 };
	mBrdfLUT = { mProgram->getUniformLocation("brdfLUT"), UniformType::TEXTURE2D, 7 };

	// shaodw mapping
	mCascadedDepthMap = { mProgram->getUniformLocation("cascadedDepthMap"), UniformType::TEXTURE2D_ARRAY, 8 };


	mEyeLightDirection = { mProgram->getUniformLocation("dirLight.directionEye"), UniformType::VEC3 };
	mLightColor = { mProgram->getUniformLocation("dirLight.color"), UniformType::VEC3 };
	mLightPower = { mProgram->getUniformLocation("dirLight.power"), UniformType::FLOAT };
	mAmbientLightPower = { mProgram->getUniformLocation("ambientLightPower"), UniformType::FLOAT };
	mShadowStrength = { mProgram->getUniformLocation("shadowStrength"), UniformType::FLOAT };

	mInverseView = { mProgram->getUniformLocation("inverseViewMatrix"), UniformType::MAT4 };

	mNearFarPlane = { mProgram->getUniformLocation("nearFarPlane"), UniformType::VEC2 };


}

void pbr::CommonLightingMaterial::setBrdfLookupTexture(const Texture* brdfLUT)
{
	mProgram->setTexture(mBrdfLUT.location, brdfLUT, mBrdfLUT.bindingSlot);
}

void pbr::CommonLightingMaterial::setIrradianceMap(const CubeMap* irradianceMap)
{
	mProgram->setTexture(mIrradianceMap.location, irradianceMap, mIrradianceMap.bindingSlot);
}

void pbr::CommonLightingMaterial::setPrefilterMap(const CubeMap* prefilterMap)
{
	mProgram->setTexture(mPrefilterMap.location, prefilterMap, mPrefilterMap.bindingSlot);
}

void pbr::CommonLightingMaterial::setCascadedDepthMap(const Texture* cascadedDepthMap)
{
	mProgram->setTexture(mCascadedDepthMap.location, cascadedDepthMap, mCascadedDepthMap.bindingSlot);
}

void pbr::CommonLightingMaterial::setCascadedData(const CascadedShadow::CascadeData& cascadedData, Camera* camera)
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

void pbr::CommonLightingMaterial::setCascadedData(ShaderStorageBuffer* buffer)
{
	buffer->bind(0);
	//buffer->syncWithGPU();
	//uniformBuffer->map(ShaderBuffer::Access::READ_WRITE);
	//uniformBuffer->unmap();
}

void pbr::CommonLightingMaterial::setEyeLightDirection(const glm::vec3& direction)
{
	mProgram->setVec3(mEyeLightDirection.location, direction);
}

void pbr::CommonLightingMaterial::setLightColor(const glm::vec3& color)
{
	mProgram->setVec3(mLightColor.location, color);
}

void pbr::CommonLightingMaterial::setLightPower(float power)
{
	mProgram->setFloat(mLightPower.location, power);
}

void pbr::CommonLightingMaterial::setAmbientLightPower(float power)
{
	mProgram->setFloat(mAmbientLightPower.location, power);
}

void pbr::CommonLightingMaterial::setShadowStrength(float strength)
{
	mProgram->setFloat(mShadowStrength.location, strength);
}

void pbr::CommonLightingMaterial::setInverseViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseView.location, mat);
}

void pbr::CommonLightingMaterial::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mProgram->setVec2(mNearFarPlane.location, nearFarPlane);
}

PBRShader::PBRShader(const CascadedShadow& cascadedShadow) : Shader(ShaderProgram::create(
																	"pbr/pbr_forward_vs.glsl", "pbr/pbr_forward_fs.glsl", "",
																	cascadedShadow.generateCsmDefines())),
                                                             CommonLightingMaterial(cascadedShadow)
{
	CommonGeometryMaterial::init(Shader::mProgram.get());
	CommonLightingMaterial::init(Shader::mProgram.get());

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

void PBRShader::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	doModelMatrixUpdate(modelMatrix);
}

void PBRShader::onMaterialUpdate(const Material* materialSource)
{
	const PbrMaterial* material = dynamic_cast<const PbrMaterial*>(materialSource);

	if (material == nullptr)
		return;

	setAlbedoMap(material->getAlbedoMap());
	setAmbientOcclusionMap(material->getAoMap());
	setMetalMap(material->getMetallicMap());
	setNormalMap(material->getNormalMap());
	setRoughnessMap(material->getRoughnessMap());
}

PBRShader_Deferred_Lighting::PBRShader_Deferred_Lighting(const CascadedShadow& cascadedShadow) : CommonLightingMaterial(cascadedShadow)
{

	Shader::mProgram = ShaderProgram::create(
		"pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs_optimized.glsl", "", cascadedShadow.generateCsmDefines());

	CommonLightingMaterial::init(Shader::mProgram.get());

	mTransform = { Shader::mProgram->getUniformLocation("transform"), UniformType::MAT4 };


	mAlbedoMap = { Shader::mProgram->getUniformLocation("gBuffer.albedoMap"), UniformType::TEXTURE2D, 0 };

	mAoMetalRoughnessMap = { Shader::mProgram->getUniformLocation("gBuffer.aoMetalRoughnessMap"), UniformType::TEXTURE2D, 1 };

	mNormalEyeMap = { Shader::mProgram->getUniformLocation("gBuffer.normalEyeMap"), UniformType::TEXTURE2D, 2 };
	mNormalizedViewSpaceZMap = { Shader::mProgram->getUniformLocation("gBuffer.normalizedViewSpaceZMap"), UniformType::TEXTURE2D, 3 };


	mInverseProjFromGPass = { Shader::mProgram->getUniformLocation("inverseProjMatrix_GPass"), UniformType::MAT4 };
}

void PBRShader_Deferred_Lighting::setMVP(const glm::mat4& trafo)
{
	Shader::mProgram->setMat4(mTransform.location, trafo);
}

void PBRShader_Deferred_Lighting::setAlbedoMap(const Texture* texture)
{
	Shader::mProgram->setTexture(mAlbedoMap.location, texture, mAlbedoMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setAoMetalRoughnessMap(const Texture* texture)
{
	Shader::mProgram->setTexture(mAoMetalRoughnessMap.location, texture, mAoMetalRoughnessMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setNormalEyeMap(const Texture* texture)
{
	Shader::mProgram->setTexture(mNormalEyeMap.location, texture, mNormalEyeMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setNormalizedViewSpaceZMap(const Texture* texture)
{
	Shader::mProgram->setTexture(mNormalizedViewSpaceZMap.location, texture, mNormalizedViewSpaceZMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setInverseProjMatrixFromGPass(const glm::mat4& mat)
{
	Shader::mProgram->setMat4(mInverseProjFromGPass.location, mat);
}

void PBRShader_Deferred_Lighting::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection) * (*data.view) * (*data.model));
}


PBRShader_Deferred_Geometry::PBRShader_Deferred_Geometry()
{
	Shader::mProgram = ShaderProgram::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl", "pbr/pbr_deferred_geometry_pass_fs.glsl");

	init(Shader::mProgram.get());
}

void PBRShader_Deferred_Geometry::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	doModelMatrixUpdate(modelMatrix);
}

void PBRShader_Deferred_Geometry::onMaterialUpdate(const Material* materialSource)
{
	const PbrMaterial* material = reinterpret_cast<const PbrMaterial*>(materialSource);

	if (material == nullptr)
		return;

	setAlbedoMap(material->getAlbedoMap());
	setAmbientOcclusionMap(material->getAoMap());
	//setEmissionMap(material->getEmissionMap());
	setMetalMap(material->getMetallicMap());
	setNormalMap(material->getNormalMap());
	setRoughnessMap(material->getRoughnessMap());
}

PBR_ConvolutionShader::PBR_ConvolutionShader()
{
	mProgram = ShaderProgram::create(
		"pbr/pbr_convolution_vs.glsl", "pbr/pbr_convolution_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mEnvironmentMap = { mProgram->getUniformLocation("environmentMap"), UniformType::CUBE_MAP, 0};
}

void PBR_ConvolutionShader::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PBR_ConvolutionShader::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void PBR_ConvolutionShader::setEnvironmentMap(const CubeMap * cubeMap)
{
	mProgram->setTexture(mEnvironmentMap.location, cubeMap, mEnvironmentMap.bindingSlot);
}

PBR_PrefilterShader::PBR_PrefilterShader()
{
	mProgram = ShaderProgram::create(
		"pbr/pbr_prefilter_cubemap_vs.glsl", "pbr/pbr_prefilter_cubemap_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mEnvironmentMap = { mProgram->getUniformLocation("environmentMap"), UniformType::CUBE_MAP, 0};
	mRoughness = { mProgram->getUniformLocation("roughness"), UniformType::FLOAT };
}

void PBR_PrefilterShader::setMapToPrefilter(CubeMap * cubeMap)
{
	mProgram->setTexture(mEnvironmentMap.location, cubeMap, mEnvironmentMap.bindingSlot);
}

void PBR_PrefilterShader::setRoughness(float roughness)
{
	mProgram->setFloat(mRoughness.location, roughness);
}

void PBR_PrefilterShader::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PBR_PrefilterShader::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

PBR_BrdfPrecomputeShader::PBR_BrdfPrecomputeShader()
{
	mProgram = ShaderProgram::create(
		"pbr/pbr_brdf_precompute_vs.glsl", "pbr/pbr_brdf_precompute_fs.glsl");

	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
}

void PBR_BrdfPrecomputeShader::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void PBR_BrdfPrecomputeShader::onTransformUpdate(const TransformData& data)
{
	setMVP(*data.projection * (*data.view) * (*data.model));
}