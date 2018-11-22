#include <nex/opengl/shader/PBRShaderGL.hpp>
#include <glm/glm.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>
#include <nex/opengl/material/PbrMaterial.hpp>

using namespace glm;
using namespace std;

PBRShaderGL::PBRShaderGL()
{
	mProgram = new ShaderProgramGL(
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

void PBRShaderGL::setAlbedoMap(const TextureGL* texture)
{
	mProgram->setTexture(mAlbedoMap.location, texture, mAlbedoMap.textureUnit);
}

void PBRShaderGL::setAmbientOcclusionMap(const TextureGL* texture)
{
	mProgram->setTexture(mAmbientOcclusionMap.location, texture, mAmbientOcclusionMap.textureUnit);
}

void PBRShaderGL::setEmissionMap(const TextureGL* texture)
{
	mProgram->setTexture(mEmissionMap.location, texture, mEmissionMap.textureUnit);
}

void PBRShaderGL::setMetalMap(const TextureGL* texture)
{
	mProgram->setTexture(mMetalMap.location, texture, mMetalMap.textureUnit);
}

void PBRShaderGL::setNormalMap(const TextureGL* texture)
{
	mProgram->setTexture(mNormalMap.location, texture, mNormalMap.textureUnit);
}

void PBRShaderGL::setRoughnessMap(const TextureGL* texture)
{
	mProgram->setTexture(mRoughnessMap.location, texture, mRoughnessMap.textureUnit);
}

void PBRShaderGL::setBrdfLookupTexture(const TextureGL* brdfLUT)
{
	mProgram->setTexture(mBrdfLUT.location, brdfLUT, mBrdfLUT.textureUnit);
}

void PBRShaderGL::setIrradianceMap(const CubeMapGL* irradianceMap)
{
	mProgram->setTexture(mIrradianceMap.location, irradianceMap, mIrradianceMap.textureUnit);
}

void PBRShaderGL::setLightColor(const glm::vec3& color)
{
	mProgram->setVec3(mLightColor.location, color);
}

void PBRShaderGL::setLightDirection(const glm::vec3& direction)
{
	mProgram->setVec3(mLightDirection.location, direction);
}

void PBRShaderGL::setLightProjMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mLightProjMatrix.location, mat);
}

void PBRShaderGL::setLightSpaceMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mLightSpaceMatrix.location, mat);
}

void PBRShaderGL::setLightViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mLightViewMatrix.location, mat);
}

void PBRShaderGL::setPrefilterMap(const CubeMapGL* prefilterMap)
{
	mProgram->setTexture(mPrefilterMap.location, prefilterMap, mPrefilterMap.textureUnit);
}

void PBRShaderGL::setProjectionMatrix(const glm::mat4& mat)
{
	mProjectionMatrixSource = &mat;
}

void PBRShaderGL::setShadowMap(const TextureGL* texture)
{
	mProgram->setTexture(mShadowMap.location, texture, mShadowMap.textureUnit);
}

void PBRShaderGL::setCameraPosition(const glm::vec3& position)
{
	mProgram->setVec3(mCameraPos.location, position);
}

void PBRShaderGL::setBiasMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mBiasMatrix.location, mat);
}

void PBRShaderGL::setModelMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mModelMatrix.location, mat);
}

void PBRShaderGL::setModelViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mModelView.location, mat);
}

void PBRShaderGL::setNormalMatrix(const glm::mat3& mat)
{
	mProgram->setMat4(mNormalMatrix.location, mat);
}

void PBRShaderGL::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void PBRShaderGL::setViewMatrix(const glm::mat4& mat)
{
	mViewMatrixSource = &mat;
	mProgram->setMat4(mView.location, mat);
}

void PBRShaderGL::setInverseViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseView.location, mat);
}

void PBRShaderGL::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	mat4 modelView = *mViewMatrixSource * modelMatrix;

	setModelMatrix(modelMatrix);
	setModelViewMatrix(modelView);
	setMVP(*mProjectionMatrixSource * modelView);
	setNormalMatrix(transpose(inverse(mat3(modelView))));
}

void PBRShaderGL::onMaterialUpdate(const Material* materialSource)
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

PBRShader_Deferred_LightingGL::PBRShader_Deferred_LightingGL()
{
	mProgram = new ShaderProgramGL(
		"pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs.glsl");

	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
	mEyeToLightTrafo = { mProgram->getUniformLocation("eyeToLight"), UniformType::MAT4 };
	mInverseViewFromGPass = { mProgram->getUniformLocation("inverseViewMatrix_GPass"), UniformType::MAT4 };
	mViewGPass = { mProgram->getUniformLocation("viewGPass"), UniformType::MAT4 };


	mEyeLightDirection = { mProgram->getUniformLocation("dirLight.directionEye"), UniformType::VEC3 };
	mLightColor = { mProgram->getUniformLocation("dirLight.color"), UniformType::VEC3 };

	mAlbedoMap = { mProgram->getUniformLocation("gBuffer.albedoMap"), UniformType::TEXTURE2D, 0 };
	mAoMetalRoughnessMap = { mProgram->getUniformLocation("gBuffer.aoMetalRoughnessMap"), UniformType::TEXTURE2D, 1 };
	mNormalEyeMap = { mProgram->getUniformLocation("gBuffer.normalEyeMap"), UniformType::TEXTURE2D, 2 };
	mPositionEyeMap = { mProgram->getUniformLocation("gBuffer.positionEyeMap"), UniformType::TEXTURE2D, 3 };

	mShadowMap = { mProgram->getUniformLocation("shadowMap"), UniformType::TEXTURE2D, 4 };
	mAoMap = { mProgram->getUniformLocation("ssaoMap"), UniformType::TEXTURE2D, 5 };

	mIrradianceMap = { mProgram->getUniformLocation("irradianceMap"), UniformType::CUBE_MAP, 6 };
	mPrefilterMap = { mProgram->getUniformLocation("prefilterMap"), UniformType::CUBE_MAP, 7 };
	mBrdfLUT = { mProgram->getUniformLocation("brdfLUT"), UniformType::TEXTURE2D, 8 };

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


	mCascadedDepthMap = { mProgram->getUniformLocation("cascadedDepthMap"), UniformType::TEXTURE2D_ARRAY, 9 };

	glCreateBuffers(1, &cascadeBufferUBO);
	glNamedBufferStorage(cascadeBufferUBO, sizeof(CascadedShadowGL::CascadeData), NULL, GL_DYNAMIC_STORAGE_BIT);
}

PBRShader_Deferred_LightingGL::~PBRShader_Deferred_LightingGL()
{
	glDeleteBuffers(1, &cascadeBufferUBO);
}

void PBRShader_Deferred_LightingGL::setMVP(const glm::mat4& trafo)
{
	mProgram->setMat4(mTransform.location, trafo);
}

void PBRShader_Deferred_LightingGL::setViewGPass(const glm::mat4& mat)
{
	mProgram->setMat4(mViewGPass.location, mat);
}

void PBRShader_Deferred_LightingGL::setInverseViewFromGPass(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseViewFromGPass.location, mat);
}

void PBRShader_Deferred_LightingGL::setBrdfLookupTexture(const TextureGL* brdfLUT)
{
	mProgram->setTexture(mBrdfLUT.location, brdfLUT, mBrdfLUT.textureUnit);
}

void PBRShader_Deferred_LightingGL::setWorldLightDirection(const glm::vec3& direction)
{
	//mProgram->setVec3(mWorldDirection.location, direction);
}

void PBRShader_Deferred_LightingGL::setEyeLightDirection(const glm::vec3& direction)
{
	mProgram->setVec3(mEyeLightDirection.location, direction);
}

void PBRShader_Deferred_LightingGL::setLightColor(const glm::vec3& color)
{
	mProgram->setVec3(mLightColor.location, color);
}

void PBRShader_Deferred_LightingGL::setIrradianceMap(const CubeMapGL* irradianceMap)
{
	mProgram->setTexture(mIrradianceMap.location, irradianceMap, mIrradianceMap.textureUnit);
}

void PBRShader_Deferred_LightingGL::setPrefilterMap(const CubeMapGL* prefilterMap)
{
	mProgram->setTexture(mPrefilterMap.location, prefilterMap, mPrefilterMap.textureUnit);
}

void PBRShader_Deferred_LightingGL::setEyeToLightSpaceMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mEyeToLightTrafo.location, mat);
}

void PBRShader_Deferred_LightingGL::setWorldToLightSpaceMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mWorldToLightTrafo.location, mat);
}

void PBRShader_Deferred_LightingGL::setShadowMap(const TextureGL* texture)
{
	mProgram->setTexture(mShadowMap.location, texture, mShadowMap.textureUnit);
}

void PBRShader_Deferred_LightingGL::setAOMap(const TextureGL* texture)
{
	mProgram->setTexture(mAoMap.location, texture, mAoMap.textureUnit);
}

void PBRShader_Deferred_LightingGL::setSkyBox(const CubeMapGL* sky)
{
	mProgram->setTexture(mSkyBox.location, sky, mSkyBox.textureUnit);
}

void PBRShader_Deferred_LightingGL::setCascadedDepthMap(const TextureGL* cascadedDepthMap)
{
	mProgram->setTexture(mCascadedDepthMap.location, cascadedDepthMap, mCascadedDepthMap.textureUnit);
}

void PBRShader_Deferred_LightingGL::setAlbedoMap(const TextureGL* texture)
{
	mProgram->setTexture(mAlbedoMap.location, texture, mAlbedoMap.textureUnit);
}

void PBRShader_Deferred_LightingGL::setAoMetalRoughnessMap(const TextureGL* texture)
{
	mProgram->setTexture(mAoMetalRoughnessMap.location, texture, mAoMetalRoughnessMap.textureUnit);
}

void PBRShader_Deferred_LightingGL::setNormalEyeMap(const TextureGL* texture)
{
	mProgram->setTexture(mNormalEyeMap.location, texture, mNormalEyeMap.textureUnit);
}

void PBRShader_Deferred_LightingGL::setPositionEyeMap(const TextureGL* texture)
{
	mProgram->setTexture(mPositionEyeMap.location, texture, mPositionEyeMap.textureUnit);
}

void PBRShader_Deferred_LightingGL::onTransformUpdate(const TransformData& data)
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


void PBRShader_Deferred_LightingGL::setCascadedData(const CascadedShadowGL::CascadeData* cascadedData)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cascadeBufferUBO);
	glNamedBufferSubData(cascadeBufferUBO, 0, sizeof(CascadedShadowGL::CascadeData), cascadedData);
}


PBRShader_Deferred_GeometryGL::PBRShader_Deferred_GeometryGL(): mProjection(nullptr), mView(nullptr)
{
	mProgram = new ShaderProgramGL(
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
}

void PBRShader_Deferred_GeometryGL::setAlbedoMap(const TextureGL* texture)
{
	mProgram->setTexture(mAlbedoMap.location, texture, mAlbedoMap.textureUnit);
}

void PBRShader_Deferred_GeometryGL::setAmbientOcclusionMap(const TextureGL* texture)
{
	mProgram->setTexture(mAmbientOcclusionMap.location, texture, mAmbientOcclusionMap.textureUnit);
}

void PBRShader_Deferred_GeometryGL::setEmissionMap(const TextureGL* texture)
{
	mProgram->setTexture(mEmissionMap.location, texture, mEmissionMap.textureUnit);
}

void PBRShader_Deferred_GeometryGL::setMetalMap(const TextureGL* texture)
{
	mProgram->setTexture(mMetalMap.location, texture, mMetalMap.textureUnit);
}

void PBRShader_Deferred_GeometryGL::setNormalMap(const TextureGL* texture)
{
	mProgram->setTexture(mNormalMap.location, texture, mNormalMap.textureUnit);
}

void PBRShader_Deferred_GeometryGL::setRoughnessMap(const TextureGL* texture)
{
	mProgram->setTexture(mRoughnessMap.location, texture, mRoughnessMap.textureUnit);
}

void PBRShader_Deferred_GeometryGL::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void PBRShader_Deferred_GeometryGL::setModelViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mModelView.location, mat);
}

void PBRShader_Deferred_GeometryGL::setModelView_NormalMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mModelView_normalMatrix.location, mat);
}

void PBRShader_Deferred_GeometryGL::setProjection(const glm::mat4& mat)
{
	mProjection = &mat;
}

void PBRShader_Deferred_GeometryGL::setView(const glm::mat4& mat)
{
	mView = &mat;
}

void PBRShader_Deferred_GeometryGL::onModelMatrixUpdate(const glm::mat4 & modelMatrix)
{
	mat4 modelView = *mView * modelMatrix;

	setModelViewMatrix(modelView);
	setMVP(*mProjection * modelView);
	setModelView_NormalMatrix(transpose(inverse(mat3(modelView))));
}

void PBRShader_Deferred_GeometryGL::onMaterialUpdate(const Material* materialSource)
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


PBR_ConvolutionShaderGL::PBR_ConvolutionShaderGL()
{
	mProgram = new ShaderProgramGL(
		"pbr/pbr_convolution_vs.glsl", "pbr/pbr_convolution_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mEnvironmentMap = { mProgram->getUniformLocation("environmentMap"), UniformType::CUBE_MAP, 0};
}

void PBR_ConvolutionShaderGL::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PBR_ConvolutionShaderGL::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

void PBR_ConvolutionShaderGL::setEnvironmentMap(const CubeMapGL * cubeMap)
{
	mProgram->setTexture(mEnvironmentMap.location, cubeMap, mEnvironmentMap.textureUnit);
}

PBR_PrefilterShaderGL::PBR_PrefilterShaderGL()
{
	mProgram = new ShaderProgramGL(
		"pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs.glsl");

	mProjection = { mProgram->getUniformLocation("projection"), UniformType::MAT4 };
	mView = { mProgram->getUniformLocation("view"), UniformType::MAT4 };
	mEnvironmentMap = { mProgram->getUniformLocation("environmentMap"), UniformType::CUBE_MAP, 0};
	mRoughness = { mProgram->getUniformLocation("roughness"), UniformType::FLOAT };
}

void PBR_PrefilterShaderGL::setMapToPrefilter(CubeMapGL * cubeMap)
{
	mProgram->setTexture(mEnvironmentMap.location, cubeMap, mEnvironmentMap.textureUnit);
}

void PBR_PrefilterShaderGL::setRoughness(float roughness)
{
	mProgram->setFloat(mRoughness.location, roughness);
}

void PBR_PrefilterShaderGL::setProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mProjection.location, mat);
}

void PBR_PrefilterShaderGL::setView(const glm::mat4& mat)
{
	mProgram->setMat4(mView.location, mat);
}

PBR_BrdfPrecomputeShaderGL::PBR_BrdfPrecomputeShaderGL()
{
	mProgram = new ShaderProgramGL(
		"pbr/pbr_deferred_lighting_pass_vs.glsl", "pbr/pbr_deferred_lighting_pass_fs.glsl");

	mTransform = { mProgram->getUniformLocation("transform"), UniformType::MAT4 };
}

void PBR_BrdfPrecomputeShaderGL::setMVP(const glm::mat4& mat)
{
	mProgram->setMat4(mTransform.location, mat);
}

void PBR_BrdfPrecomputeShaderGL::onTransformUpdate(const TransformData& data)
{
	setMVP(*data.projection * (*data.view) * (*data.model));
}