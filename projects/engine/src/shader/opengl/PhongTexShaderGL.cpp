#include <shader/opengl/PhongTexShaderGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <texture/opengl/TextureManagerGL.hpp>
#include <glad/glad.h>

using namespace glm;
using namespace std;

PhongTexShaderGL::PhongTexShaderGL() : dirLightDirection(1, 0, 0), lightColor(1, 1, 1), 
pointLightRange(0), pointLightShadowMap(nullptr), shadowMap(nullptr), skybox(nullptr), 
viewPosition(0,0,0), vsMap(nullptr)
{

}

PhongTexShaderGL::~PhongTexShaderGL() {}

const vec3& PhongTexShaderGL::getLightColor() const
{
	return lightColor;
}

const vec3& PhongTexShaderGL::getLightPosition() const
{
	return dirLightDirection;
}

void PhongTexShaderGL::setLightColor(vec3 color)
{
	lightColor = color;
}

void PhongTexShaderGL::setLightDirection(vec3 direction)
{
	dirLightDirection = direction;
}

void PhongTexShaderGL::setLightSpaceMatrix(mat4 mat)
{
	lightSpaceMatrix = move(mat);
}

void PhongTexShaderGL::setPointLightPositions(vec3* positions)
{
	for (int i = 0; i < 4; ++i)
	{
		pointLightPositions[i] = positions[i];
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
}

void PhongTexShaderGL::setShadowMap(Texture* texture)
{
	shadowMap = dynamic_cast<TextureGL*>(texture);
	assert(shadowMap != nullptr);
}

void PhongTexShaderGL::setSkyBox(CubeMap* sky)
{
	this->skybox = static_cast<CubeMapGL*>(sky);
}

void PhongTexShaderGL::setSpotLightDirection(vec3 direction)
{
	spotLightDirection = move(direction);
}

void PhongTexShaderGL::setVarianceShadowMap(Texture* texture)
{ 
	vsMap = dynamic_cast<TextureGL*>(texture);
	assert(vsMap != nullptr);
}

void PhongTexShaderGL::initLights(GLuint programID)
{
	// == ==========================
	// Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index 
	// the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
	// by defining light types as classes and set their values in there, or by using a more efficient uniform approach
	// by using 'Uniform buffer objects', but that is something we discuss in the 'Advanced GLSL' tutorial.
	// == ==========================
	// Directional light
	glUniform3f(glGetUniformLocation(programID, "dirLight.direction"), dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
	glUniform4f(glGetUniformLocation(programID, "dirLight.ambient"), 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "dirLight.diffuse"), 0.5f, 0.5f, 0.5f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "dirLight.specular"), 0.3f, 0.3f, 0.3f, 1.0f);

	for (int i = 0; i < 4; ++i)
	{
		string pointLightStr = "pointLights[" + to_string(i) + "]";
		GLuint loc = glGetUniformLocation(programID, (pointLightStr + ".position").c_str());
		glUniform3f(loc, pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
		glUniform4f(glGetUniformLocation(programID, (pointLightStr + ".ambient").c_str()), 0.05f, 0.05f, 0.05f, 1.0f);
		glUniform4f(glGetUniformLocation(programID, (pointLightStr + ".diffuse").c_str()), 0.8f, 0.8f, 0.8f, 1.0f);
		glUniform4f(glGetUniformLocation(programID, (pointLightStr + ".specular").c_str()), 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(programID, (pointLightStr + ".constant").c_str()), 1.0f);
		glUniform1f(glGetUniformLocation(programID, (pointLightStr + ".linear").c_str()), 0.09f);
		glUniform1f(glGetUniformLocation(programID, (pointLightStr + ".quadratic").c_str()), 0.032f);
	}
	// SpotLight
	glUniform3f(glGetUniformLocation(programID, "spotLight.position"), viewPosition.x, viewPosition.y, viewPosition.z);
	glUniform3f(glGetUniformLocation(programID, "spotLight.direction"), spotLightDirection.x, spotLightDirection.y, spotLightDirection.z);
	glUniform4f(glGetUniformLocation(programID, "spotLight.ambient"), 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "spotLight.diffuse"), 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "spotLight.specular"), 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(programID, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "spotLight.linear"), 0.09f);
	glUniform1f(glGetUniformLocation(programID, "spotLight.quadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(programID, "spotLight.cutOff"), cos(radians(12.5f)));
	glUniform1f(glGetUniformLocation(programID, "spotLight.outerCutOff"), cos(radians(15.0f)));
}

void PhongTexShaderGL::initForDrawing(Mesh const& meshOriginal, GLuint programID)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& model = *data.model;

	initLights(programID);

	// set model material data
	//GLint matAmbientLoc = glGetUniformLocation(programID, "material.ambient");
	GLint matShineLoc = glGetUniformLocation(programID, "material.shininess");

	//const vec3& ambient = material->getAmbient();
	const Material& material = mesh.getMaterial();

	TextureGL* diffuseMap = static_cast<TextureGL*>(material.getDiffuseMap());
	TextureGL* reflectionMap = static_cast<TextureGL*>(material.getReflectionMap());
	TextureGL* specularMap = static_cast<TextureGL*>(material.getSpecularMap());
	TextureGL* emissionMap = static_cast<TextureGL*>(material.getEmissionMap());

	//glUniform3f(matAmbientLoc, ambient.x, ambient.y, ambient.z);
	glUniform1f(matShineLoc, material.getShininess());

	// Bind diffuse map
	if (diffuseMap)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap->getTexture());
		glUniform1i(glGetUniformLocation(getProgramID(), "material.diffuseMap"), 0);
	}
	else {
		GLuint textureID = TextureManagerGL::get()->getImageGL("black.png")->getTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(programID, "material.diffuseMap"), 0);
	}

	// Bind emission map
	if (emissionMap)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, emissionMap->getTexture());
		glUniform1i(glGetUniformLocation(programID, "material.emissionMap"), 1);
	}
	else {
		GLuint textureID = TextureManagerGL::get()->getImageGL("black.png")->getTexture();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(programID, "material.emissionMap"), 1);
	}

	// Bind reflection map
	if (reflectionMap)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, reflectionMap->getTexture());
		glUniform1i(glGetUniformLocation(programID, "material.reflectionMap"), 2);
	}
	else
	{
		GLuint textureID = TextureManagerGL::get()->getImageGL("black.png")->getTexture();
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(programID, "material.reflectionMap"), 2);
	}

	// Bind specular map
	if (specularMap)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, specularMap->getTexture());
		glUniform1i(glGetUniformLocation(programID, "material.specularMap"), 3);
	}
	else {
		GLuint textureID = TextureManagerGL::get()->getImageGL("black.png")->getTexture();
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(programID, "material.specularMap"), 3);
	}

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, shadowMap->getTexture());
	glUniform1i(glGetUniformLocation(programID, "material.shadowMap"), 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->getCubeMap());
	glUniform1i(glGetUniformLocation(programID, "skybox"), 5);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, pointLightShadowMap->getCubeMapTexture());
	glUniform1i(glGetUniformLocation(programID, "cubeDepthMap"), 6);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, vsMap->getTexture());
	glUniform1i(glGetUniformLocation(programID, "material.vsMap"), 7);
}

void PhongTexShaderGL::setViewPosition(vec3 position)
{
	viewPosition = move(position);
}

void PhongTexShaderGL::update(const MeshGL& mesh, const TransformData& data)
{
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	transform = projection * view * model;
	modelView = view * model;
	normalMatrix = transpose(inverse(model));
	static mat4 biasMatrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
		);

	attributes.setData("projection", (void*)data.projection);
	attributes.setData("view", (void*)data.view);
	attributes.setData("transform", &transform);
	attributes.setData("modelView", &modelView);
	attributes.setData("normalMatrix", &normalMatrix);
	attributes.setData("biasMatrix", &biasMatrix);
	attributes.setData("lightSpaceMatrix", &lightSpaceMatrix);
	attributes.setData("range", &pointLightRange);

	attributes.setData("viewPos", &viewPosition);
}