#include <nex/opengl/shader/PBRShaderGL.hpp>
#include <glm/glm.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/material/PbrMaterial.hpp>

using namespace glm;
using namespace std;

PBRShaderGL::PBRShaderGL() : lightColor(1, 1, 1), shadowMap(nullptr), skybox(nullptr), cameraPos(0,0,0)
{
	using types = ShaderAttributeType;

	//attributes.create(types::MAT4, nullptr, "projection");
	//attributes.create(types::MAT4, nullptr, "view");
	attributes.create(types::MAT4, &transform, "transform", true);
	attributes.create(types::MAT4, &modelMatrix, "model", true);
	attributes.create(types::MAT4, nullptr, "view", true);
	attributes.create(types::MAT4, nullptr, "inverseViewMatrix", true);
	attributes.create(types::MAT4, &modelView, "modelView", true);
	attributes.create(types::MAT3, &normalMatrix, "normalMatrix", true);

	biasMatrix = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	attributes.create(types::MAT4, &biasMatrix, "biasMatrix", true);
	attributes.create(types::MAT4, &lightSpaceMatrix, "eyeToLightSpaceMatrix", true);
	attributes.create(types::MAT4, &lightProjMatrix, "lightProjMatrix", true);
	attributes.create(types::MAT4, &lightViewMatrix, "lightViewMatrix", true);

	attributes.create(types::VEC3, &cameraPos, "cameraPos", true);


	dirLight.color = { 0.5f, 0.5f, 0.5f};
	dirLight.direction = { 0,1,0 };

	attributes.create(types::VEC3, &dirLight.direction, "dirLight.direction", true);
	attributes.create(types::VEC4, &dirLight.color, "dirLight.color", true);

	//attributes.create(types::FLOAT, &pointLightRange, "range", true);

	attributes.create(types::TEXTURE2D, nullptr, "material.albedoMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.aoMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.metallicMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.normalMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.roughnessMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.shadowMap");

	attributes.create(types::CUBE_MAP, nullptr, "irradianceMap");
	attributes.create(types::CUBE_MAP, nullptr, "prefilterMap");
	attributes.create(types::TEXTURE2D, nullptr, "brdfLUT");


	//attributes.create(types::CUBE_MAP, nullptr, "skybox");
}

PBRShaderGL::~PBRShaderGL() {}

const vec3& PBRShaderGL::getLightColor() const
{
	return lightColor;
}

const vec3& PBRShaderGL::getLightPosition() const
{
	return dirLight.direction;
}

void PBRShaderGL::setBrdfLookupTexture(Texture * brdfLUT)
{
	this->brdfLUT = dynamic_cast<TextureGL*>(brdfLUT);
	attributes.setData("brdfLUT", this->brdfLUT);
}

void PBRShaderGL::setIrradianceMap(CubeMap * irradianceMap)
{
	this->irradianceMap = dynamic_cast<CubeMapGL*>(irradianceMap);
	attributes.setData("irradianceMap", this->irradianceMap);
}

void PBRShaderGL::setLightColor(vec3 color)
{
	lightColor = color;
}

void PBRShaderGL::setLightDirection(vec3 direction)
{
	dirLight.direction = move(direction);
}

void PBRShaderGL::setLightProjMatrix(mat4 mat)
{
	lightProjMatrix = move(mat);
}

void PBRShaderGL::setLightSpaceMatrix(mat4 mat)
{
	lightSpaceMatrix = move(mat);
}

void PBRShaderGL::setLightViewMatrix(glm::mat4 mat)
{
	lightViewMatrix = move(mat);
}

void PBRShaderGL::setPrefilterMap(CubeMap* prefilterMap) {
	this->prefilterMap = dynamic_cast<CubeMapGL*>(prefilterMap);
	attributes.setData("prefilterMap", this->prefilterMap);
}



void PBRShaderGL::setShadowMap(Texture* texture)
{
	shadowMap = dynamic_cast<TextureGL*>(texture);
	assert(shadowMap != nullptr);
	Texture* black = TextureManagerGL::get()->getDefaultBlackTexture();
	attributes.setData("material.shadowMap", shadowMap, black);
}

void PBRShaderGL::setSkyBox(CubeMap* sky)
{
	this->skybox = dynamic_cast<CubeMapGL*>(sky);
	//attributes.setData("skybox", dynamic_cast<CubeMapGL*>(skybox));
}

void PBRShaderGL::setCameraPosition(vec3 position)
{
	cameraPos = move(position);
}



void PBRShaderGL::update(const MeshGL& mesh, const TransformData& data)
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

	PbrMaterial* material = dynamic_cast<PbrMaterial*>(&mesh.getMaterial().get());

	TextureGL* albedoMap = static_cast<TextureGL*>(material->getAlbedoMap());
	TextureGL* aoMap = static_cast<TextureGL*>(material->getAoMap());
	TextureGL* emissionMap = static_cast<TextureGL*>(material->getEmissionMap());
	TextureGL* metallicMap = static_cast<TextureGL*>(material->getMetallicMap());
	TextureGL* normalMap = static_cast<TextureGL*>(material->getNormalMap());
	TextureGL* roughnessMap = static_cast<TextureGL*>(material->getRoughnessMap());

	TextureGL* black = static_cast<TextureGL*>(TextureManagerGL::get()->getDefaultBlackTexture());
	TextureGL* white = static_cast<TextureGL*>(TextureManagerGL::get()->getDefaultWhiteTexture());
	TextureGL* default_normal = static_cast<TextureGL*>(TextureManagerGL::get()->getDefaultNormalTexture()); //brickwall_normal

	attributes.setData("material.albedoMap", albedoMap, black);

	attributes.setData("material.aoMap", aoMap, white);
	//attributes.setData("material.emissionMap", emissionMap, black);
	attributes.setData("material.metallicMap", metallicMap, black);
	attributes.setData("material.normalMap", normalMap, default_normal);
	attributes.setData("material.roughnessMap", roughnessMap, black);

	//attributes.setData("brdfLUT", white, white);
}

PBRShader_Deferred_LightingGL::PBRShader_Deferred_LightingGL() : PBRShader_Deferred_Lighting(), ShaderConfigGL()
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
}

PBRShader_Deferred_LightingGL::~PBRShader_Deferred_LightingGL()
{
}

void PBRShader_Deferred_LightingGL::setBrdfLookupTexture(Texture * brdfLUT)
{
	this->brdfLUT = dynamic_cast<TextureGL*>(brdfLUT);
	attributes.setData("brdfLUT", this->brdfLUT);
}

void PBRShader_Deferred_LightingGL::setGBuffer(PBR_GBuffer * gBuffer)
{
	this->gBuffer = gBuffer;
}

void PBRShader_Deferred_LightingGL::setInverseViewFromGPass(glm::mat4 inverseView)
{
	this->inverseViewFromGPass = move(inverseView);
	attributes.setData("inverseViewMatrix_GPass", &this->inverseViewFromGPass);
}

void PBRShader_Deferred_LightingGL::setIrradianceMap(CubeMap * irradianceMap)
{
	this->irradianceMap = dynamic_cast<CubeMapGL*>(irradianceMap);
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


void PBRShader_Deferred_LightingGL::setPrefilterMap(CubeMap * prefilterMap)
{
	this->prefilterMap = dynamic_cast<CubeMapGL*>(prefilterMap);
	attributes.setData("prefilterMap", this->prefilterMap);
}

void PBRShader_Deferred_LightingGL::setShadowMap(Texture * texture)
{
	shadowMap = dynamic_cast<TextureGL*>(texture);
	assert(shadowMap != nullptr);
	Texture* black = TextureManagerGL::get()->getDefaultBlackTexture();
	attributes.setData("shadowMap", shadowMap);
}

void PBRShader_Deferred_LightingGL::setSSAOMap(Texture * texture)
{
	ssaoMap = dynamic_cast<TextureGL*>(texture);
	assert(ssaoMap != nullptr);
	attributes.setData("ssaoMap", ssaoMap);
}

void PBRShader_Deferred_LightingGL::setSkyBox(CubeMap * sky)
{
	this->skybox = dynamic_cast<CubeMapGL*>(sky);
	//attributes.setData("skybox", dynamic_cast<CubeMapGL*>(skybox));
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
}


PBRShader_Deferred_GeometryGL::PBRShader_Deferred_GeometryGL() : PBRShader_Deferred_Geometry(), ShaderConfigGL()
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

	PbrMaterial* material = dynamic_cast<PbrMaterial*>(&mesh.getMaterial().get());

	TextureGL* albedoMap = static_cast<TextureGL*>(material->getAlbedoMap());
	TextureGL* aoMap = static_cast<TextureGL*>(material->getAoMap());
	TextureGL* emissionMap = static_cast<TextureGL*>(material->getEmissionMap());
	TextureGL* metallicMap = static_cast<TextureGL*>(material->getMetallicMap());
	TextureGL* normalMap = static_cast<TextureGL*>(material->getNormalMap());
	TextureGL* roughnessMap = static_cast<TextureGL*>(material->getRoughnessMap());

	TextureGL* black = static_cast<TextureGL*>(TextureManagerGL::get()->getDefaultBlackTexture());
	TextureGL* white = static_cast<TextureGL*>(TextureManagerGL::get()->getDefaultWhiteTexture());
	TextureGL* default_normal = static_cast<TextureGL*>(TextureManagerGL::get()->getDefaultNormalTexture()); //brickwall_normal

	attributes.setData("material.albedoMap", albedoMap, black); 
	attributes.setData("material.aoMap", aoMap, white);
	//attributes.setData("material.emissionMap", emissionMap, black);
	attributes.setData("material.metallicMap", metallicMap, black);
	attributes.setData("material.normalMap", normalMap, default_normal);
	attributes.setData("material.roughnessMap", roughnessMap, black);

	//glBindSampler(albedoMap->getTexture(), m_sampler.getID());
	/*for (int i = 0; i < 32; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, albedoMap->getTexture());
		//glSamplerParameterf(m_sampler.getID(), GL_TEXTURE_MAX_ANISOTROPY, 16.0f);
		glBindSampler(i, m_sampler.getID());
		//float value = 16.0f;
		//glSamplerParameterfv(m_sampler.getID(), GL_TEXTURE_MAX_ANISOTROPY, &value);
		//glSamplerParameterf(m_sampler.getID(), GL_TEXTURE_MAX_ANISOTROPY, 16.0f);
		
	}*/
}

void PBRShader_Deferred_GeometryGL::beforeDrawing(const MeshGL& mesh)
{
}

void PBRShader_Deferred_GeometryGL::afterDrawing(const MeshGL& mesh)
{
	/*for (int i = 0; i < 32; ++i)
	{
		glBindSampler(i, GL_FALSE);
	}*/
	/*glBindSampler(1, GL_FALSE);
	glBindSampler(2, GL_FALSE);
	glBindSampler(3, GL_FALSE);
	glBindSampler(4, GL_FALSE);*/
}


PBR_ConvolutionShaderGL::PBR_ConvolutionShaderGL() : ShaderConfigGL(), PBR_ConvolutionShader()
{
	attributes.create(ShaderAttributeType::CUBE_MAP, nullptr, "environmentMap");
	attributes.create(ShaderAttributeType::MAT4, nullptr, "projection", true);
	attributes.create(ShaderAttributeType::MAT4, nullptr, "view", true);
}

PBR_ConvolutionShaderGL::~PBR_ConvolutionShaderGL()
{
}

void PBR_ConvolutionShaderGL::setEnvironmentMap(CubeMap * cubeMap)
{
	this->cubeMap = dynamic_cast<CubeMapGL*>(cubeMap);
	attributes.setData("environmentMap", dynamic_cast<CubeMapGL*>(cubeMap));
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

void PBR_PrefilterShaderGL::setMapToPrefilter(CubeMap * cubeMap)
{
	this->cubeMap = dynamic_cast<CubeMapGL*>(cubeMap);
	attributes.setData("environmentMap", dynamic_cast<CubeMapGL*>(cubeMap));
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