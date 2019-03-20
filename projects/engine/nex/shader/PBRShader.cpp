#include <nex/shader/PBRShader.hpp>
#include <glm/glm.hpp>
#include <nex/mesh/SubMesh.hpp>
#include <nex/material/Material.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;
using namespace nex;

PBRShader::PBRShader()
{
	mProgram = ShaderProgram::create(
		"pbr/pbr_forward_vs.glsl", "pbr/pbr_forward_fs.glsl");

	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mModelMatrix = { mProgram->getUniformLocation("model"), UniformType::MAT4 };

	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mInverseView = { mProgram->getUniformLocation("inverseViewMatrix"), UniformType::MAT4 };

	mModelView = { mProgram->getUniformLocation("modelView"), UniformType::MAT4 };
	mNormalMatrix = { mProgram->getUniformLocation("normalMatrix"), UniformType::MAT3 };

	mBiasMatrix = { mProgram->getUniformLocation("biasMatrix"), UniformType::MAT4 };
	mBiasMatrixSource = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	bind();
	mProgram->setMat4(mBiasMatrix.location, mBiasMatrixSource);
	unbind();


	mLightSpaceMatrix = { mProgram->getUniformLocation("eyeToLightSpaceMatrix"), UniformType::MAT4 };
	mLightProjMatrix = { mProgram->getUniformLocation("lightProjMatrix"), UniformType::MAT4 };
	mLightViewMatrix = { mProgram->getUniformLocation("lightViewMatrix"), UniformType::MAT4 };

	mCameraPos = { mProgram->getUniformLocation("cameraPos"), UniformType::VEC3 };

	mLightDirection = { mProgram->getUniformLocation("dirLight.direction"), UniformType::VEC3 };
	mLightColor = { mProgram->getUniformLocation("dirLight.color"), UniformType::VEC3 };



	mAlbedoMap = { mProgram->getUniformLocation("material.albedoMap"), UniformType::TEXTURE2D, 0};
	mAmbientOcclusionMap = { mProgram->getUniformLocation("material.aoMap"), UniformType::TEXTURE2D, 1};
	mEmissionMap = { mProgram->getUniformLocation("material.emissionMap"), UniformType::TEXTURE2D, 2};
	mMetalMap = { mProgram->getUniformLocation("material.metallicMap"), UniformType::TEXTURE2D, 3};
	mNormalMap = { mProgram->getUniformLocation("material.normalMap"), UniformType::TEXTURE2D, 4};
	mRoughnessMap = { mProgram->getUniformLocation("material.roughnessMap"), UniformType::TEXTURE2D, 5};


	mShadowMap = { mProgram->getUniformLocation("material.shadowMap"), UniformType::TEXTURE2D, 6};

	mIrradianceMap = { mProgram->getUniformLocation("irradianceMap"), UniformType::CUBE_MAP, 7};
	mPrefilterMap = { mProgram->getUniformLocation("prefilterMap"), UniformType::CUBE_MAP, 8};

	mBrdfLUT = { mProgram->getUniformLocation("brdfLUT"), UniformType::TEXTURE2D, 9};

	//attributes.create(types::CUBE_MAP, nullptr, "skybox");
}

void PBRShader::setAlbedoMap(const Texture* texture)
{
	mProgram->setTexture(mAlbedoMap.location, texture, mAlbedoMap.bindingSlot);
}

void PBRShader::setAmbientOcclusionMap(const Texture* texture)
{
	mProgram->setTexture(mAmbientOcclusionMap.location, texture, mAmbientOcclusionMap.bindingSlot);
}

void PBRShader::setEmissionMap(const Texture* texture)
{
	mProgram->setTexture(mEmissionMap.location, texture, mEmissionMap.bindingSlot);
}

void PBRShader::setMetalMap(const Texture* texture)
{
	mProgram->setTexture(mMetalMap.location, texture, mMetalMap.bindingSlot);
}

void PBRShader::setNormalMap(const Texture* texture)
{
	mProgram->setTexture(mNormalMap.location, texture, mNormalMap.bindingSlot);
}

void PBRShader::setRoughnessMap(const Texture* texture)
{
	mProgram->setTexture(mRoughnessMap.location, texture, mRoughnessMap.bindingSlot);
}

void PBRShader::setBrdfLookupTexture(const Texture* brdfLUT)
{
	mProgram->setTexture(mBrdfLUT.location, brdfLUT, mBrdfLUT.bindingSlot);
}

void PBRShader::setIrradianceMap(const CubeMap* irradianceMap)
{
	mProgram->setTexture(mIrradianceMap.location, irradianceMap, mIrradianceMap.bindingSlot);
}

void PBRShader::setLightColor(const glm::vec3& color)
{
	mProgram->setVec3(mLightColor.location, color);
}

void PBRShader::setLightDirection(const glm::vec3& direction)
{
	mProgram->setVec3(mLightDirection.location, direction);
}

void PBRShader::setLightProjMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mLightProjMatrix.location, mat);
}

void PBRShader::setLightSpaceMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mLightSpaceMatrix.location, mat);
}

void PBRShader::setLightViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mLightViewMatrix.location, mat);
}

void PBRShader::setPrefilterMap(const CubeMap* prefilterMap)
{
	mProgram->setTexture(mPrefilterMap.location, prefilterMap, mPrefilterMap.bindingSlot);
}

void PBRShader::setProjectionMatrix(const glm::mat4& mat)
{
	mProjectionMatrixSource = &mat;
}

void PBRShader::setShadowMap(const Texture* texture)
{
	mProgram->setTexture(mShadowMap.location, texture, mShadowMap.bindingSlot);
}

void PBRShader::setCameraPosition(const glm::vec3& position)
{
	mProgram->setVec3(mCameraPos.location, position);
}

void PBRShader::setBiasMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mBiasMatrix.location, mat);
}

void PBRShader::setModelMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mModelMatrix.location, mat);
}

void PBRShader::setModelViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mModelView.location, mat);
}

void PBRShader::setNormalMatrix(const glm::mat3& mat)
{
	mProgram->setMat4(mNormalMatrix.location, mat);
}

void PBRShader::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void PBRShader::setViewMatrix(const glm::mat4& mat)
{
	mViewMatrixSource = &mat;
	mProgram->setMat4(mView.location, mat);
}

void PBRShader::setInverseViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseView.location, mat);
}

void PBRShader::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	mat4 modelView = *mViewMatrixSource * modelMatrix;

	setModelMatrix(modelMatrix);
	setModelViewMatrix(modelView);
	setMVP(*mProjectionMatrixSource * modelView);
	setNormalMatrix(transpose(inverse(mat3(modelView))));
}

void PBRShader::onMaterialUpdate(const Material* materialSource)
{
	const PbrMaterial* material = dynamic_cast<const PbrMaterial*>(materialSource);

	if (material == nullptr)
		return;

	setAlbedoMap(material->getAlbedoMap());
	setAmbientOcclusionMap(material->getAoMap());
	setEmissionMap(material->getEmissionMap());
	setMetalMap(material->getMetallicMap());
	setNormalMap(material->getNormalMap());
	setRoughnessMap(material->getRoughnessMap());
}


// TODO
/*void PBRShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	modelMatrix = model;

	inverseView = inverse(view);

	transform = projection * view * model;
	modelView = view * model;
	normalMatrix = transpose(inverse(mat3(modelView)));
	
	//attributes.setData("projection", data.projection);
	attributes.setData("model", &modelMatrix, nullptr, true);
	attributes.setData("view", data.view);
	attributes.setData("inverseViewMatrix", &inverseView);
	attributes.setData("transform", &transform);
	attributes.setData("modelView", &modelView);
	attributes.setData("normalMatrix", &normalMatrix);

	PbrMaterial* material = dynamic_cast<PbrMaterial*>(mesh.getMaterial());

	TextureGL* albedoMap = material->getAlbedoMap();
	TextureGL* aoMap = material->getAoMap();
	TextureGL* emissionMap = material->getEmissionMap();
	TextureGL* metallicMap = material->getMetallicMap();
	TextureGL* normalMap = material->getNormalMap();
	TextureGL* roughnessMap = material->getRoughnessMap();


	attributes.setData("material.albedoMap", albedoMap);

	attributes.setData("material.aoMap", aoMap);
	//attributes.setData("material.emissionMap", emissionMap, black);
	attributes.setData("material.metallicMap", metallicMap);
	attributes.setData("material.normalMap", normalMap);
	attributes.setData("material.roughnessMap", roughnessMap);

	//attributes.setData("brdfLUT", white, white);
}*/

PBRShader_Deferred_Lighting::PBRShader_Deferred_Lighting(const CascadedShadow& cascadedShadow) :
	cascadeBufferUBO(0, CascadedShadow::CascadeData::calcCascadeDataByteSize(cascadedShadow.getCascadeData().numCascades), ShaderBuffer::UsageHint::DYNAMIC_COPY),
	mCsmNumCascades(cascadedShadow.getCascadeData().numCascades),
	mCsmPcf(cascadedShadow.getPCF()),
	mCsmEnabled(cascadedShadow.isEnabled()),
	mCsmBiasMultiplier(cascadedShadow.getBiasMultiplier())
{

	std::vector<string> defines = generateCsmDefines();

	mProgram = ShaderProgram::create(
		"pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs.glsl", "", defines);

	unsigned textureCounter = 0;

	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mEyeToLightTrafo = { mProgram->getUniformLocation("eyeToLight"), UniformType::MAT4 };
	mInverseViewFromGPass = { mProgram->getUniformLocation("inverseViewMatrix_GPass"), UniformType::MAT4 };
	mViewGPass = { mProgram->getUniformLocation("viewGPass"), UniformType::MAT4 };


	mEyeLightDirection = { mProgram->getUniformLocation("dirLight.directionEye"), UniformType::VEC3 };
	mLightColor = { mProgram->getUniformLocation("dirLight.color"), UniformType::VEC3 };
	mLightPower = { mProgram->getUniformLocation("dirLight.power"), UniformType::FLOAT };
	mAmbientLightPower = { mProgram->getUniformLocation("ambientLightPower"), UniformType::FLOAT };
	mShadowStrength = { mProgram->getUniformLocation("shadowStrength"), UniformType::FLOAT };

	mAlbedoMap = { mProgram->getUniformLocation("gBuffer.albedoMap"), UniformType::TEXTURE2D, 0 };
	if (mAlbedoMap.location != -1)
		++textureCounter;

	mAoMetalRoughnessMap = { mProgram->getUniformLocation("gBuffer.aoMetalRoughnessMap"), UniformType::TEXTURE2D, 1 };
	if (mAoMetalRoughnessMap.location != -1)
		++textureCounter;

	mNormalEyeMap = { mProgram->getUniformLocation("gBuffer.normalEyeMap"), UniformType::TEXTURE2D, 2 };
	if (mNormalEyeMap.location != -1)
		++textureCounter;
	mNormalizedViewSpaceZMap = { mProgram->getUniformLocation("gBuffer.normalizedViewSpaceZMap"), UniformType::TEXTURE2D, 3 };
	if (mNormalizedViewSpaceZMap.location != -1)
		++textureCounter;

	//mShadowMap = { mProgram->getUniformLocation("shadowMap"), UniformType::TEXTURE2D, 5 };
	mAoMap = { mProgram->getUniformLocation("ssaoMap"), UniformType::TEXTURE2D, textureCounter };
	if (mAoMap.location != -1)
		++textureCounter;

	mIrradianceMap = { mProgram->getUniformLocation("irradianceMap"), UniformType::CUBE_MAP, textureCounter };
	if (mIrradianceMap.location != -1)
		++textureCounter;
	mPrefilterMap = { mProgram->getUniformLocation("prefilterMap"), UniformType::CUBE_MAP, textureCounter };
	if (mPrefilterMap.location != -1)
		++textureCounter;
	mBrdfLUT = { mProgram->getUniformLocation("brdfLUT"), UniformType::TEXTURE2D, textureCounter };
	if (mBrdfLUT.location != -1)
		++textureCounter;

	mBiasMatrix = { mProgram->getUniformLocation("biasMatrix"), UniformType::MAT4 };
	mBiasMatrixSource = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	bind();
	mProgram->setMat4(mBiasMatrix.location, mBiasMatrixSource);
	unbind();


	mCascadedDepthMap = { mProgram->getUniformLocation("cascadedDepthMap"), UniformType::TEXTURE2D_ARRAY, textureCounter };
	if (mCascadedDepthMap.location != -1)
		++textureCounter;

	mInverseProjFromGPass = { mProgram->getUniformLocation("inverseProjMatrix_GPass"), UniformType::MAT4 };

	mNearFarPlane = { mProgram->getUniformLocation("nearFarPlane"), UniformType::VEC2 };

	//glCreateBuffers(1, &cascadeBufferUBO);
	//glNamedBufferStorage(cascadeBufferUBO, sizeof(CascadedShadowGL::CascadeData), NULL, GL_DYNAMIC_STORAGE_BIT);
}

PBRShader_Deferred_Lighting::~PBRShader_Deferred_Lighting()
{
	//glDeleteBuffers(1, &cascadeBufferUBO);
}

void PBRShader_Deferred_Lighting::setMVP(const glm::mat4& trafo)
{
	mProgram->setMat4(mTransform.location, trafo);
}

void PBRShader_Deferred_Lighting::setViewGPass(const glm::mat4& mat)
{
	mProgram->setMat4(mViewGPass.location, mat);
}

void PBRShader_Deferred_Lighting::setInverseViewFromGPass(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseViewFromGPass.location, mat);
}

void PBRShader_Deferred_Lighting::setBrdfLookupTexture(const Texture* brdfLUT)
{
	mProgram->setTexture(mBrdfLUT.location, brdfLUT, mBrdfLUT.bindingSlot);
}

void PBRShader_Deferred_Lighting::setWorldLightDirection(const glm::vec3& direction)
{
	//mProgram->setVec3(mWorldDirection.location, direction);
}

void PBRShader_Deferred_Lighting::setEyeLightDirection(const glm::vec3& direction)
{
	mProgram->setVec3(mEyeLightDirection.location, direction);
}

void PBRShader_Deferred_Lighting::setLightColor(const glm::vec3& color)
{
	mProgram->setVec3(mLightColor.location, color);
}

void PBRShader_Deferred_Lighting::setLightPower(float power)
{
	mProgram->setFloat(mLightPower.location, power);
}

void PBRShader_Deferred_Lighting::setAmbientLightPower(float power)
{
	mProgram->setFloat(mAmbientLightPower.location, power);
}

void PBRShader_Deferred_Lighting::setShadowStrength(float strength)
{
	mProgram->setFloat(mShadowStrength.location, strength);
}

void PBRShader_Deferred_Lighting::setIrradianceMap(const CubeMap* irradianceMap)
{
	mProgram->setTexture(mIrradianceMap.location, irradianceMap, mIrradianceMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setPrefilterMap(const CubeMap* prefilterMap)
{
	mProgram->setTexture(mPrefilterMap.location, prefilterMap, mPrefilterMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setEyeToLightSpaceMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mEyeToLightTrafo.location, mat);
}

void PBRShader_Deferred_Lighting::setWorldToLightSpaceMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mWorldToLightTrafo.location, mat);
}

void PBRShader_Deferred_Lighting::setShadowMap(const Texture* texture)
{
	mProgram->setTexture(mShadowMap.location, texture, mShadowMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setAOMap(const Texture* texture)
{
	mProgram->setTexture(mAoMap.location, texture, mAoMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setSkyBox(const CubeMap* sky)
{
	mProgram->setTexture(mSkyBox.location, sky, mSkyBox.bindingSlot);
}

void PBRShader_Deferred_Lighting::setCascadedDepthMap(const Texture* cascadedDepthMap)
{
	mProgram->setTexture(mCascadedDepthMap.location, cascadedDepthMap, mCascadedDepthMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setCascadedData(ShaderStorageBuffer* buffer)
{
	auto* uniformBuffer = (ShaderStorageBuffer*)buffer; //UniformBuffer ShaderStorageBuffer
	//uniformBuffer->unbind();
	uniformBuffer->bind(0);
	uniformBuffer->syncWithGPU();
	//uniformBuffer->map(ShaderBuffer::Access::READ_WRITE);
	//uniformBuffer->unmap();
	
}

void PBRShader_Deferred_Lighting::setAlbedoMap(const Texture* texture)
{
	mProgram->setTexture(mAlbedoMap.location, texture, mAlbedoMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setAoMetalRoughnessMap(const Texture* texture)
{
	mProgram->setTexture(mAoMetalRoughnessMap.location, texture, mAoMetalRoughnessMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setNormalEyeMap(const Texture* texture)
{
	mProgram->setTexture(mNormalEyeMap.location, texture, mNormalEyeMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setNormalizedViewSpaceZMap(const Texture* texture)
{
	mProgram->setTexture(mNormalizedViewSpaceZMap.location, texture, mNormalizedViewSpaceZMap.bindingSlot);
}

void PBRShader_Deferred_Lighting::setInverseProjMatrixFromGPass(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseProjFromGPass.location, mat);
}

void PBRShader_Deferred_Lighting::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mProgram->setVec2(mNearFarPlane.location, nearFarPlane);
}

void PBRShader_Deferred_Lighting::onTransformUpdate(const TransformData& data)
{
	setMVP((*data.projection) * (*data.view) * (*data.model));
}

//TODO

/*
void PBRShader_Deferred_LightingGL::update(const MeshGL & mesh, const TransformData & data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;
	eyeToLight = worldToLight * inverseViewFromGPass;
	myView = inverse(inverseViewFromGPass);

	dirEyeToLight.color = dirWorldToLight.color;
	dirEyeToLight.direction = vec3(myView * vec4(dirWorldToLight.direction, 0));


	
	//myView = mat4();
	attributes.setData("viewGPass", &myView);
	attributes.setData("inverseViewMatrix_GPass", &inverseViewFromGPass);
	attributes.setData("transform", &transform);
	attributes.setData("eyeToLight", &eyeToLight);

	attributes.setData("shadowMap", shadowMap);
	attributes.setData("ssaoMap", ssaoMap);

	attributes.setData("gBuffer.albedoMap", gBuffer->getAlbedo());
	attributes.setData("gBuffer.aoMetalRoughnessMap", gBuffer->getAoMetalRoughness());
	attributes.setData("gBuffer.normalEyeMap", gBuffer->getNormal());
	attributes.setData("gBuffer.positionEyeMap", gBuffer->getPosition());

	attributes.setData("dirLight.color", &dirEyeToLight.color);
	attributes.setData("dirLight.directionEye", &dirEyeToLight.direction);

	attributes.setData("irradianceMap", irradianceMap);
	attributes.setData("prefilterMap", prefilterMap);
	attributes.setData("brdfLUT", brdfLUT);

	attributes.setData("cascadedDepthMap", this->cascadedDepthMap);
}*/


void PBRShader_Deferred_Lighting::setCascadedData(const CascadedShadow::CascadeData& cascadedData, Camera* camera)
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


PBRShader_Deferred_Geometry::PBRShader_Deferred_Geometry(): mProjection(nullptr), mView(nullptr)
{
	mProgram = ShaderProgram::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl", "pbr/pbr_deferred_geometry_pass_fs.glsl");

	mTransform = {mProgram->getUniformLocation("transform"), UniformType::MAT4};
	mModelView = {mProgram->getUniformLocation("modelView"), UniformType::MAT4};
	mModelView_normalMatrix = {mProgram->getUniformLocation("modelView_normalMatrix"), UniformType::MAT4};

	mAlbedoMap = {mProgram->getUniformLocation("material.albedoMap"), UniformType::TEXTURE2D, 0};
	mAmbientOcclusionMap = {mProgram->getUniformLocation("material.aoMap"), UniformType::TEXTURE2D, 1};

	// TODO
	mEmissionMap = {mProgram->getUniformLocation("material.emissionMap"), UniformType::TEXTURE2D, 2};

	mMetalMap = {mProgram->getUniformLocation("material.metallicMap"), UniformType::TEXTURE2D, 3};
	mNormalMap = {mProgram->getUniformLocation("material.normalMap"), UniformType::TEXTURE2D, 4};
	mRoughnessMap = {mProgram->getUniformLocation("material.roughnessMap"), UniformType::TEXTURE2D, 5};

	mNearFarPlane = { mProgram->getUniformLocation("nearFarPlane"), UniformType::VEC2 };
}

void PBRShader_Deferred_Geometry::setAlbedoMap(const Texture* texture)
{
	mProgram->setTexture(mAlbedoMap.location, texture, mAlbedoMap.bindingSlot);
}

void PBRShader_Deferred_Geometry::setAmbientOcclusionMap(const Texture* texture)
{
	mProgram->setTexture(mAmbientOcclusionMap.location, texture, mAmbientOcclusionMap.bindingSlot);
}

void PBRShader_Deferred_Geometry::setEmissionMap(const Texture* texture)
{
	mProgram->setTexture(mEmissionMap.location, texture, mEmissionMap.bindingSlot);
}

void PBRShader_Deferred_Geometry::setMetalMap(const Texture* texture)
{
	mProgram->setTexture(mMetalMap.location, texture, mMetalMap.bindingSlot);
}

void PBRShader_Deferred_Geometry::setNormalMap(const Texture* texture)
{
	mProgram->setTexture(mNormalMap.location, texture, mNormalMap.bindingSlot);
}

void PBRShader_Deferred_Geometry::setRoughnessMap(const Texture* texture)
{
	mProgram->setTexture(mRoughnessMap.location, texture, mRoughnessMap.bindingSlot);
}

void PBRShader_Deferred_Geometry::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void PBRShader_Deferred_Geometry::setModelViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mModelView.location, mat);
}

void PBRShader_Deferred_Geometry::setModelView_NormalMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mModelView_normalMatrix.location, mat);
}

void PBRShader_Deferred_Geometry::setProjection(const glm::mat4& mat)
{
	mProjection = &mat;
}

void PBRShader_Deferred_Geometry::setView(const glm::mat4& mat)
{
	mView = &mat;
}

void PBRShader_Deferred_Geometry::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	mat4 modelView = *mView * modelMatrix;

	setModelViewMatrix(modelView);
	setMVP(*mProjection * modelView);
	setModelView_NormalMatrix(transpose(inverse(mat3(modelView))));
}

void PBRShader_Deferred_Geometry::onMaterialUpdate(const Material* materialSource)
{
	const PbrMaterial* material = reinterpret_cast<const PbrMaterial*>(materialSource);

	if (material == nullptr)
		return;

	setAlbedoMap(material->getAlbedoMap());
	setAmbientOcclusionMap(material->getAoMap());
	setEmissionMap(material->getEmissionMap());
	setMetalMap(material->getMetallicMap());
	setNormalMap(material->getNormalMap());
	setRoughnessMap(material->getRoughnessMap());
}

void PBRShader_Deferred_Geometry::setNearFarPlane(const glm::vec2& nearFarPlane)
{
	mProgram->setVec2(mNearFarPlane.location, nearFarPlane);
}

//TODO
/*
void PBRShader_Deferred_GeometryGL::update(const MeshGL & mesh, const TransformData & data)
{

	//glSamplerParameteri(m_sampler.getID(), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glSamplerParameteri(m_sampler.getID(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glSamplerParameterf(m_sampler.getID(), GL_TEXTURE_MAX_ANISOTROPY, 4.0f);

	//float value = 0.0f;
	//glGetSamplerParameterfv(m_sampler.getID(), GL_TEXTURE_MAX_ANISOTROPY, &value);

	//std::cout << "GL_TEXTURE_MAX_ANISOTROPY = " << value << std::endl;

	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;
	modelView = view * model;
	modelView_normalMatrix = mat3(transpose(inverse(modelView)));

	//attributes.setData("projection", data.projection);
	attributes.setData("transform", &transform);
	attributes.setData("modelView", &modelView);
	attributes.setData("modelView_normalMatrix", &modelView_normalMatrix);

	PbrMaterial* material = dynamic_cast<PbrMaterial*>(mesh.getMaterial());

	TextureGL* albedoMap = material->getAlbedoMap();
	TextureGL* aoMap = material->getAoMap();
	TextureGL* emissionMap = material->getEmissionMap();
	TextureGL* metallicMap = material->getMetallicMap();
	TextureGL* normalMap = material->getNormalMap();
	TextureGL* roughnessMap = material->getRoughnessMap();

	attributes.setData("material.albedoMap", albedoMap); 
	attributes.setData("material.aoMap", aoMap);
	//attributes.setData("material.emissionMap", emissionMap, black);
	attributes.setData("material.metallicMap", metallicMap);
	attributes.setData("material.normalMap", normalMap);
	attributes.setData("material.roughnessMap", roughnessMap);
}
*/


std::vector<std::string> PBRShader_Deferred_Lighting::generateCsmDefines()
{
	std::vector<string> result;

	result.emplace_back(makeDefine("CSM_NUM_CASCADES", mCsmNumCascades));
	result.emplace_back(makeDefine("CSM_SAMPLE_COUNT_X", mCsmPcf.sampleCountX));
	result.emplace_back(makeDefine("CSM_SAMPLE_COUNT_Y", mCsmPcf.sampleCountY));
	result.emplace_back(makeDefine("CSM_USE_LERP_FILTER", mCsmPcf.useLerpFiltering));
	result.emplace_back(makeDefine("CSM_ENABLED", mCsmEnabled));
	result.emplace_back(makeDefine("CSM_BIAS_MULTIPLIER", mCsmBiasMultiplier));

	return result;
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