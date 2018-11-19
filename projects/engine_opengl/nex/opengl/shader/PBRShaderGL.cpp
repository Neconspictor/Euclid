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

void PBRShaderGL::setInverseViewMatrix(const glm::mat4& mat)
{
	mProgram->setMat4(mInverseView.location, mat);
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

PBRShader_Deferred_LightingGL::PBRShader_Deferred_LightingGL() : ShaderConfigGL()
{
	using types = ShaderAttributeType;

	attributes.create(types::MAT4, &transform, "transform", true);
	attributes.create(types::MAT4, &eyeToLight, "eyeToLight", true);
	attributes.create(types::MAT4, &inverseViewFromGPass, "inverseViewMatrix_GPass", true);
	attributes.create(types::MAT4, &myView, "viewGPass", true);

	dirWorldToLight.color = { 0.5f, 0.5f, 0.5f };
	dirWorldToLight.direction = { 0,1,0 };

	attributes.create(types::VEC3, nullptr, "dirLight.directionEye");
	attributes.create(types::VEC4, nullptr, "dirLight.color");

	attributes.create(types::TEXTURE2D, nullptr, "gBuffer.albedoMap");
	attributes.create(types::TEXTURE2D, nullptr, "gBuffer.aoMetalRoughnessMap");
	attributes.create(types::TEXTURE2D, nullptr, "gBuffer.normalEyeMap");
	attributes.create(types::TEXTURE2D, nullptr, "gBuffer.positionEyeMap");

	attributes.create(types::TEXTURE2D, nullptr, "shadowMap");
	attributes.create(types::TEXTURE2D, nullptr, "ssaoMap");

	attributes.create(types::CUBE_MAP, nullptr, "irradianceMap");
	attributes.create(types::CUBE_MAP, nullptr, "prefilterMap");
	attributes.create(types::TEXTURE2D, nullptr, "brdfLUT");

	biasMatrix = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
		);

	attributes.create(types::TEXTURE2D_ARRAY, nullptr, "cascadedDepthMap");

	glCreateBuffers(1, &cascadeBufferUBO);
	glNamedBufferStorage(cascadeBufferUBO, sizeof(CascadedShadowGL::CascadeData), NULL, GL_DYNAMIC_STORAGE_BIT);
}

PBRShader_Deferred_LightingGL::~PBRShader_Deferred_LightingGL()
{
	glDeleteBuffers(1, &cascadeBufferUBO);
}

void PBRShader_Deferred_LightingGL::setBrdfLookupTexture(TextureGL * brdfLUT)
{
	this->brdfLUT = brdfLUT;
	attributes.setData("brdfLUT", this->brdfLUT);
}

void PBRShader_Deferred_LightingGL::setGBuffer(PBR_GBufferGL * gBuffer)
{
	this->gBuffer = gBuffer;
}

void PBRShader_Deferred_LightingGL::setInverseViewFromGPass(glm::mat4 inverseView)
{
	this->inverseViewFromGPass = move(inverseView);
	attributes.setData("inverseViewMatrix_GPass", &this->inverseViewFromGPass);
}

void PBRShader_Deferred_LightingGL::setIrradianceMap(CubeMapGL * irradianceMap)
{
	this->irradianceMap = irradianceMap;
	attributes.setData("irradianceMap", this->irradianceMap);
}

void PBRShader_Deferred_LightingGL::setLightColor(glm::vec3 color)
{
	this->dirWorldToLight.color = move(color);
}

void PBRShader_Deferred_LightingGL::setLightDirection(glm::vec3 direction)
{
	this->dirWorldToLight.direction = move(direction);
}


void PBRShader_Deferred_LightingGL::setPrefilterMap(CubeMapGL * prefilterMap)
{
	this->prefilterMap = prefilterMap;
	attributes.setData("prefilterMap", this->prefilterMap);
}

void PBRShader_Deferred_LightingGL::setShadowMap(TextureGL * texture)
{
	shadowMap = texture;
	assert(shadowMap != nullptr);
	attributes.setData("shadowMap", shadowMap);
}

void PBRShader_Deferred_LightingGL::setAOMap(TextureGL * texture)
{
	ssaoMap = texture;
	assert(ssaoMap != nullptr);
	attributes.setData("ssaoMap", ssaoMap);
}

void PBRShader_Deferred_LightingGL::setSkyBox(CubeMapGL * sky)
{
	this->skybox = sky;
	//attributes.setData("skybox", dynamic_cast<CubeMapGL*>(skybox));
	//TODO IMPORTANT
}

void PBRShader_Deferred_LightingGL::setWorldToLightSpaceMatrix(glm::mat4 worldToLight)
{
	this->worldToLight = move(worldToLight);
}


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
}

void PBRShader_Deferred_LightingGL::setCascadedDepthMap(TextureGL* cascadedDepthMap)
{
	this->cascadedDepthMap = cascadedDepthMap;
}

void PBRShader_Deferred_LightingGL::setCascadedData(CascadedShadowGL::CascadeData* cascadedData)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cascadeBufferUBO);
	glNamedBufferSubData(cascadeBufferUBO, 0, sizeof(CascadedShadowGL::CascadeData), cascadedData);
}


PBRShader_Deferred_GeometryGL::PBRShader_Deferred_GeometryGL() : ShaderConfigGL()
{
	using types = ShaderAttributeType;

	attributes.create(types::MAT4, &transform, "transform", true);
	attributes.create(types::MAT4, &modelView, "modelView", true);
	attributes.create(types::MAT3, &modelView_normalMatrix, "modelView_normalMatrix", true);

	attributes.create(types::TEXTURE2D, nullptr, "material.albedoMap"); //Texture0
	attributes.create(types::TEXTURE2D, nullptr, "material.aoMap"); //Texture1
	attributes.create(types::TEXTURE2D, nullptr, "material.metallicMap"); //Texture2
	attributes.create(types::TEXTURE2D, nullptr, "material.normalMap"); //Texture3
	attributes.create(types::TEXTURE2D, nullptr, "material.roughnessMap"); //Texture4

	//GLuint sampler;
	//glGenSamplers(1, &sampler);
	/*glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_LOD_BIAS, 0);*/

	/*glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);*/

	//float anisotropy = renderer->getTextureManager()->getAnisotropicFiltering();

	//glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

	//m_sampler.setID(sampler);
}

PBRShader_Deferred_GeometryGL::~PBRShader_Deferred_GeometryGL()
{
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


PBR_ConvolutionShaderGL::PBR_ConvolutionShaderGL() : ShaderConfigGL()
{
	attributes.create(ShaderAttributeType::CUBE_MAP, nullptr, "environmentMap");
	attributes.create(ShaderAttributeType::MAT4, nullptr, "projection", true);
	attributes.create(ShaderAttributeType::MAT4, nullptr, "view", true);
}

PBR_ConvolutionShaderGL::~PBR_ConvolutionShaderGL()
{
}

void PBR_ConvolutionShaderGL::setEnvironmentMap(CubeMapGL * cubeMap)
{
	this->cubeMap = cubeMap;
	attributes.setData("environmentMap", cubeMap);
}

void PBR_ConvolutionShaderGL::update(const MeshGL & mesh, const TransformData & data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	attributes.setData("projection", &projection);
	attributes.setData("view", &view);
}

PBR_PrefilterShaderGL::PBR_PrefilterShaderGL() : roughness(0.0)
{
	attributes.create(ShaderAttributeType::CUBE_MAP, nullptr, "environmentMap");
	attributes.create(ShaderAttributeType::FLOAT, &this->roughness, "roughness", true);
	attributes.create(ShaderAttributeType::MAT4, nullptr, "projection", true);
	attributes.create(ShaderAttributeType::MAT4, nullptr, "view", true);
}

PBR_PrefilterShaderGL::~PBR_PrefilterShaderGL()
{
}

void PBR_PrefilterShaderGL::setMapToPrefilter(CubeMapGL * cubeMap)
{
	this->cubeMap = cubeMap;
	attributes.setData("environmentMap", cubeMap);
}

void PBR_PrefilterShaderGL::setRoughness(float roughness)
{
	this->roughness = roughness;
}

void PBR_PrefilterShaderGL::update(const MeshGL & mesh, const TransformData & data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	attributes.setData("projection", &projection);
	attributes.setData("view", &view);
}

PBR_BrdfPrecomputeShaderGL::PBR_BrdfPrecomputeShaderGL() : transform(mat4())
{
	//attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
	attributes.create(ShaderAttributeType::FLOAT, nullptr, "test");

	attributes.create(ShaderAttributeType::MAT4, &transform, "transform", true);
}

PBR_BrdfPrecomputeShaderGL::~PBR_BrdfPrecomputeShaderGL()
{
}

void PBR_BrdfPrecomputeShaderGL::update(const MeshGL & mesh, const TransformData & data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;

	float test = 6.6f;

	//attributes.setData("transform", &transform);
	attributes.setData("test", &test);
}