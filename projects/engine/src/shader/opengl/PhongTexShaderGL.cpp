#include <shader/opengl/PhongTexShaderGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <texture/opengl/TextureManagerGL.hpp>
#include <GL/glew.h>

using namespace glm;
using namespace std;

PhongTexShaderGL::PhongTexShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile) :
	ShaderGL(vertexShaderFile, fragmentShaderFile), lightColor(1, 1, 1), lightPosition(1, 0, 0),
	material(nullptr), viewPosition(0,0,0)
{
}

PhongTexShaderGL::~PhongTexShaderGL()
{
}

void PhongTexShaderGL::draw(Model const& model, mat4 const& projection, mat4 const& view)
{
	MeshGL* mesh = getFromModel(model);
	use();
	glBindVertexArray(mesh->getVertexArrayObject());

	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(projection * view * model.getTrafo()));

	GLuint modelLoc = glGetUniformLocation(getProgramID(), "modelView");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model.getTrafo()));

	// specular color is calculated in view space; so multiply normal matrix
	// and light position by the view matrix.

	GLint normalMatrixLoc = glGetUniformLocation(getProgramID(), "normalMatrix");
	mat4 normalMatrix = transpose(inverse(model.getTrafo()));
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, value_ptr(normalMatrix));

	GLint lightPositionLoc = glGetUniformLocation(getProgramID(), "light.position");
	GLint viewPosLoc = glGetUniformLocation(getProgramID(), "viewPos");

	vec3 lightPositionViewSpace = vec4(lightPosition, 1.0f); // adding 1.0f as the fourth element is important (do not use 0.0f)!
	glUniform3f(lightPositionLoc, lightPositionViewSpace.x, lightPositionViewSpace.y, lightPositionViewSpace.z);

	// set light data
	GLint lightAmbientLoc = glGetUniformLocation(getProgramID(), "light.ambient");
	GLint lightDiffuseLoc = glGetUniformLocation(getProgramID(), "light.diffuse");
	GLint lightSpecularLoc = glGetUniformLocation(getProgramID(), "light.specular");

	glUniform3f(lightAmbientLoc, 0.1*lightColor.x, 0.1*lightColor.y, 0.1*lightColor.z);
	glUniform3f(lightDiffuseLoc, lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(lightSpecularLoc, lightColor.x, lightColor.y, lightColor.z);

	glUniform3f(viewPosLoc, viewPosition.x, viewPosition.y, viewPosition.z);

	// set model material data
	GLint matAmbientLoc = glGetUniformLocation(programID, "material.ambient");
	GLint matSpecularLoc = glGetUniformLocation(programID, "material.specular");
	GLint matShineLoc = glGetUniformLocation(programID, "material.shininess");

	const vec3& ambient = material->getAmbient();
	const string& diffuseMapStr = material->getDiffuseMap();
	GLuint diffuseMap = TextureManagerGL::get()->getImage(diffuseMapStr);
	vec3 specular = material->getSpecular();

	glUniform3f(matAmbientLoc, ambient.x, ambient.y, ambient.z);
	glUniform3f(matSpecularLoc, specular.x, specular.y, specular.z);
	glUniform1f(matShineLoc, material->getSpecularPower());

	// Bind diffuse map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glUniform1i(glGetUniformLocation(getProgramID(), "material.diffuseMap"), 0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount());
	glBindVertexArray(0);
}

const vec3& PhongTexShaderGL::getLightColor() const
{
	return lightColor;
}

const vec3& PhongTexShaderGL::getLightPosition() const
{
	return lightPosition;
}

bool PhongTexShaderGL::loadingFailed()
{
	return ShaderGL::loadingFailed();
}

void PhongTexShaderGL::release()
{
	ShaderGL::release();
}

void PhongTexShaderGL::setLightColor(vec3 color)
{
	lightColor = move(color);
}

void PhongTexShaderGL::setLightPosition(vec3 position)
{
	lightPosition = move(position);
}

void PhongTexShaderGL::setMaterial(const PhongTexMaterial& material)
{
	this->material = &material;
}

void PhongTexShaderGL::use()
{
	glUseProgram(this->programID);
}

void PhongTexShaderGL::setViewPosition(vec3 position)
{
	viewPosition = move(position);
}