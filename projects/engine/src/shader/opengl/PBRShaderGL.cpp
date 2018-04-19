#include <shader/opengl/PBRShaderGL.hpp>
#include <glm/glm.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <texture/opengl/TextureManagerGL.hpp>

using namespace glm;
using namespace std;

PBRShaderGL::PBRShaderGL() : lightColor(1, 1, 1), shadowMap(nullptr), skybox(nullptr), viewPosition(0,0,0)
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
	attributes.create(types::TEXTURE2D, nullptr, "material.normalMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.specularMap");
	attributes.create(types::TEXTURE2D, nullptr, "material.shadowMap");

	attributes.create(types::CUBE_MAP, nullptr, "skybox");
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
	attributes.setData("skybox", dynamic_cast<TextureGL*>(skybox));
}

void PBRShaderGL::setViewPosition(vec3 position)
{
	viewPosition = move(position);
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

	const Material& material = mesh.getMaterial();

	TextureGL* diffuseMap = static_cast<TextureGL*>(material.getDiffuseMap());
	TextureGL* specularMap = static_cast<TextureGL*>(material.getSpecularMap());
	TextureGL* normalMap = static_cast<TextureGL*>(material.getNormalMap());

	TextureGL* black = static_cast<TextureGL*>(TextureManagerGL::get()->getDefaultBlackTexture());
	TextureGL* default_normal = static_cast<TextureGL*>(TextureManagerGL::get()->getDefaultNormalTexture()); //brickwall_normal

	attributes.setData("material.shininess", &material.getShininessRef());
	attributes.setData("material.diffuseMap", diffuseMap, black);
	attributes.setData("material.specularMap", specularMap, black);
	attributes.setData("material.normalMap", normalMap, default_normal);
}

