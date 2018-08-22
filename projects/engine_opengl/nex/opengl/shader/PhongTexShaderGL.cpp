#include <nex/opengl/shader/PhongTexShaderGL.hpp>
#include <glm/glm.hpp>
#include <nex/opengl/mesh/MeshGL.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>

using namespace glm;
using namespace std;

PhongTexShaderGL::PhongTexShaderGL() : lightColor(1, 1, 1), 
pointLightRange(0), pointLightShadowMap(nullptr), shadowMap(nullptr), skybox(nullptr), 
viewPosition(0,0,0), vsMap(nullptr)
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

	attributes.create(types::VEC3, &viewPosition, "viewPos", true);

	dirLight.ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
	dirLight.diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
	dirLight.specular = { 0.3f, 0.3f, 0.3f, 1.0f };
	dirLight.direction = { 0,1,0 };

	attributes.create(types::VEC3, &dirLight.direction, "dirLight.direction", true);
	attributes.create(types::VEC4, &dirLight.ambient, "dirLight.ambient", true);
	attributes.create(types::VEC4, &dirLight.diffuse, "dirLight.diffuse", true);
	attributes.create(types::VEC4, &dirLight.specular, "dirLight.specular", true);

	//attributes.create(types::FLOAT, &pointLightRange, "range", true);

	attributes.create(types::FLOAT, nullptr, "material.shininess");
	attributes.create(types::TEXTURE2D, nullptr, "material.diffuseMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.emissionMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.normalMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.reflectionMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.specularMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.shadowMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.vsMap");

	attributes.create(types::CUBE_MAP, nullptr, "cubeDepthMap");
	attributes.create(types::CUBE_MAP, nullptr, "skybox");
}

PhongTexShaderGL::~PhongTexShaderGL() {}

const vec3& PhongTexShaderGL::getLightColor() const
{
	return lightColor;
}

const vec3& PhongTexShaderGL::getLightPosition() const
{
	return dirLight.direction;
}

void PhongTexShaderGL::setLightColor(vec3 color)
{
	lightColor = color;
}

void PhongTexShaderGL::setLightDirection(vec3 direction)
{
	dirLight.direction = move(direction);
}

void PhongTexShaderGL::setLightProjMatrix(mat4 mat)
{
	lightProjMatrix = move(mat);
}

void PhongTexShaderGL::setLightSpaceMatrix(mat4 mat)
{
	lightSpaceMatrix = move(mat);
}

void PhongTexShaderGL::setLightViewMatrix(glm::mat4 mat)
{
	lightViewMatrix = move(mat);
}

void PhongTexShaderGL::setPointLightPositions(vec3* positions)
{
	for (int i = 0; i < 4; ++i)
	{
		pointLights[i].position = positions[i];
	}
}

void PhongTexShaderGL::setPointLightRange(float range)
{
	this->pointLightRange = range;
}

void PhongTexShaderGL::setPointLightShadowMap(CubeDepthMap* map)
{
	pointLightShadowMap = dynamic_cast<CubeDepthMapGL*>(map);
	assert(pointLightShadowMap);
	attributes.setData("cubeDepthMap", dynamic_cast<TextureGL*>(pointLightShadowMap));
}

void PhongTexShaderGL::setShadowMap(Texture* texture)
{
	shadowMap = dynamic_cast<TextureGL*>(texture);
	assert(shadowMap != nullptr);
	TextureGL* black = TextureManagerGL::get()->getImageGL("_intern/black.png");
	attributes.setData("material.shadowMap", shadowMap, black);
}

void PhongTexShaderGL::setSkyBox(CubeMap* sky)
{
	this->skybox = dynamic_cast<CubeMapGL*>(sky);
	attributes.setData("skybox", dynamic_cast<TextureGL*>(skybox));
}

void PhongTexShaderGL::setSpotLightDirection(vec3 direction)
{
	spotLight.direction = move(direction);
}

void PhongTexShaderGL::setVarianceShadowMap(Texture* texture)
{ 
	vsMap = dynamic_cast<TextureGL*>(texture);
	assert(vsMap != nullptr);
	TextureGL* black = TextureManagerGL::get()->getImageGL("_intern/black.png");
	attributes.setData("material.vsMap", vsMap, black);
}

void PhongTexShaderGL::setViewPosition(vec3 position)
{
	viewPosition = move(position);
	spotLight.position = viewPosition;
}

void PhongTexShaderGL::update(const MeshGL& mesh, const TransformData& data)
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

	BlinnPhongMaterial* material = dynamic_cast<BlinnPhongMaterial*>(&mesh.getMaterial().get());
	TextureGL* diffuseMap = static_cast<TextureGL*>(material->getDiffuseMap());
	TextureGL* reflectionMap = static_cast<TextureGL*>(material->getReflectionMap());
	TextureGL* specularMap = static_cast<TextureGL*>(material->getSpecularMap());
	TextureGL* emissionMap = static_cast<TextureGL*>(material->getEmissionMap());
	TextureGL* normalMap = static_cast<TextureGL*>(material->getNormalMap());
	TextureGL* black = TextureManagerGL::get()->getImageGL("_intern/black.png");
	Texture* default_normal = TextureManagerGL::get()->getImage("_intern/default_normal.png", { false, true, Linear_Mipmap_Linear, Linear, Repeat, RGB }); //brickwall_normal
	//TextureGL* default_normal = TextureManagerGL::get()->getImageGL("stones/brickwall_normal.jpg"); //brickwall_normal

	attributes.setData("material.shininess", &material->getShininessRef());
	attributes.setData("material.diffuseMap", diffuseMap, black);
	attributes.setData("material.emissionMap", emissionMap, black);
	attributes.setData("material.reflectionMap", reflectionMap, black);
	attributes.setData("material.specularMap", specularMap, black);
	attributes.setData("material.normalMap", normalMap, default_normal);
}