#include <shader/opengl/PhongShaderGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace glm;
using namespace std;

PhongShaderGL::PhongShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile): 
	ShaderGL(vertexShaderFile, fragmentShaderFile), lightColor(1, 1, 1), lightPosition(1,0,0), objectColor(1, 1, 1)
{
}

PhongShaderGL::~PhongShaderGL()
{
}

void PhongShaderGL::draw(Model const& model, mat4 const& projection, mat4 const& view)
{
	MeshGL* mesh = getFromModel(model);
	use();
	glBindVertexArray(mesh->getVertexArrayObject());

	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(projection * view * model.getTrafo()));

	GLuint modelLoc = glGetUniformLocation(getProgramID(), "modelView");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(view * model.getTrafo()));

	// specular color is calculated in view space; so multiply normal matrix
	// and light position by the view matrix.

	GLint normalMatrixLoc = glGetUniformLocation(getProgramID(), "normalMatrix");
	mat4 normalMatrix = transpose(inverse(view * model.getTrafo()));
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, value_ptr(normalMatrix));

	GLint lightPositionLoc = glGetUniformLocation(getProgramID(), "lightPosition");

	vec3 lightPositionViewSpace = view * vec4(lightPosition, 1.0f); // adding 1.0f as the fourth element is important (do not use 0.0f)!
	glUniform3f(lightPositionLoc, lightPositionViewSpace.x, lightPositionViewSpace.y, lightPositionViewSpace.z);


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount());
	glBindVertexArray(0);
}

const vec3& PhongShaderGL::getLightColor() const
{
	return lightColor;
}

const vec3& PhongShaderGL::getLightPosition() const
{
	return lightPosition;
}

const vec3& PhongShaderGL::getObjectColor() const
{
	return objectColor;
}

bool PhongShaderGL::loadingFailed()
{
	return ShaderGL::loadingFailed();
}

void PhongShaderGL::release()
{
	ShaderGL::release();
}

void PhongShaderGL::setLightColor(vec3 color)
{
	lightColor = move(color);
}

void PhongShaderGL::setLightPosition(vec3 position)
{
	lightPosition = move(position);
}

void PhongShaderGL::setObjectColor(vec3 color)
{
	objectColor = move(color);
}

void PhongShaderGL::use()
{
	glUseProgram(this->programID);
	GLint objectColorLoc = glGetUniformLocation(getProgramID(), "objectColor");
	GLint lightColorLoc = glGetUniformLocation(getProgramID(), "lightColor");
	glUniform3f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z);
}