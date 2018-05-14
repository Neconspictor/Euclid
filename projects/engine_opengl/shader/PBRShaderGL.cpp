#include <shader/PBRShaderGL.hpp>
#include <glm/glm.hpp>
#include <mesh/MeshGL.hpp>
#include <texture/TextureManagerGL.hpp>
#include <material/PbrMaterial.hpp>

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
	attributes.create(types::MAT4, &modelView, "modelView", true);
	attributes.create(types::MAT3, &normalMatrix, "normalMatrix", true);

	biasMatrix = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	attributes.create(types::MAT4, &biasMatrix, "biasMatrix", true);
	attributes.create(types::MAT4, &lightSpaceMatrix, "lightSpaceMatrix", true);
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

	transform = projection * view * model;
	modelView = view * model;
	normalMatrix = transpose(inverse(mat3(modelView)));
	
	//attributes.setData("projection", data.projection);
	attributes.setData("model", &modelMatrix, nullptr, true);
	attributes.setData("view", data.view);
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

PBRShader_DeferredGL::PBRShader_DeferredGL()
{
}

PBRShader_DeferredGL::~PBRShader_DeferredGL()
{
}

const glm::vec3 & PBRShader_DeferredGL::getLightColor() const
{
	return base.getLightColor();
}

const glm::vec3 & PBRShader_DeferredGL::getLightPosition() const
{
	return base.getLightPosition();
}

void PBRShader_DeferredGL::setBrdfLookupTexture(Texture * brdfLUT)
{
	base.setBrdfLookupTexture(brdfLUT);
}

void PBRShader_DeferredGL::setIrradianceMap(CubeMap * irradianceMap)
{
	base.setIrradianceMap(irradianceMap);
}

void PBRShader_DeferredGL::setLightColor(glm::vec3 color)
{
	base.setLightColor(move(color));
}

void PBRShader_DeferredGL::setLightDirection(glm::vec3 direction)
{
	base.setLightDirection(move(direction));
}

void PBRShader_DeferredGL::setLightProjMatrix(glm::mat4 mat)
{
	base.setLightProjMatrix(move(mat));
}

void PBRShader_DeferredGL::setLightSpaceMatrix(glm::mat4 mat)
{
	base.setLightSpaceMatrix(move(mat));
}

void PBRShader_DeferredGL::setLightViewMatrix(glm::mat4 mat)
{
	base.setLightViewMatrix(move(mat));
}

void PBRShader_DeferredGL::setPrefilterMap(CubeMap * prefilterMap)
{
	base.setPrefilterMap(prefilterMap);
}

void PBRShader_DeferredGL::setShadowMap(Texture * texture)
{
	base.setShadowMap(texture);
}

void PBRShader_DeferredGL::setSkyBox(CubeMap * sky)
{
	base.setSkyBox(sky);
}

void PBRShader_DeferredGL::setCameraPosition(glm::vec3 position)
{
	base.setCameraPosition(move(position));
}

void PBRShader_DeferredGL::update(const MeshGL & mesh, const TransformData & data)
{
	base.update(mesh, data);
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

	float test = 6.6;

	//attributes.setData("transform", &transform);
	attributes.setData("test", &test);
}