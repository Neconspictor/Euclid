#include <shader/opengl/PhongShaderGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace glm;
using namespace std;

PhongShaderGL::PhongShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile): 
	ShaderGL(vertexShaderFile, fragmentShaderFile), lightColor(1, 1, 1), lightPosition(1,0,0), 
	material(nullptr)
{
}

PhongShaderGL::~PhongShaderGL()
{
}

void PhongShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = static_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	use();

	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(projection * view * model));

	GLuint modelLoc = glGetUniformLocation(getProgramID(), "modelView");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(view * model));

	// specular color is calculated in view space; so multiply normal matrix
	// and light position by the view matrix.

	GLint normalMatrixLoc = glGetUniformLocation(getProgramID(), "normalMatrix");
	mat4 normalMatrix = transpose(inverse(view * model));
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, value_ptr(normalMatrix));

	GLint lightPositionLoc = glGetUniformLocation(getProgramID(), "light.position");

	vec3 lightPositionViewSpace = view * vec4(lightPosition, 1.0f); // adding 1.0f as the fourth element is important (do not use 0.0f)!
	glUniform3f(lightPositionLoc, lightPositionViewSpace.x, lightPositionViewSpace.y, lightPositionViewSpace.z);

	// set light data
	GLint lightAmbientLoc = glGetUniformLocation(getProgramID(), "light.ambient");
	GLint lightDiffuseLoc = glGetUniformLocation(getProgramID(), "light.diffuse");
	GLint lightSpecularLoc = glGetUniformLocation(getProgramID(), "light.specular");

	glUniform3f(lightAmbientLoc, 0.1f*lightColor.x, 0.1f*lightColor.y, 0.1f*lightColor.z);
	glUniform3f(lightDiffuseLoc, lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(lightSpecularLoc, 0.5f*lightColor.x, 0.5f*lightColor.y, 0.5f*lightColor.z);

	// set model material data
	GLint matAmbientLoc = glGetUniformLocation(getProgramID(), "material.ambient");
	GLint matDiffuseLoc = glGetUniformLocation(getProgramID(), "material.diffuse");
	GLint matSpecularLoc = glGetUniformLocation(getProgramID(), "material.specular");
	GLint matShineLoc = glGetUniformLocation(getProgramID(), "material.shininess");


	const vec4& ambient = material->getAmbient();
	const vec4& diffuse = material->getDiffuse();
	const vec4& specular = material->getSpecular();

	glUniform3f(matAmbientLoc, ambient.r, ambient.g, ambient.b);
	glUniform3f(matDiffuseLoc, diffuse.r, diffuse.g, diffuse.b);
	glUniform3f(matSpecularLoc, specular.r, specular.g, specular.b);
	glUniform1f(matShineLoc, material->getSpecularPower());

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void PhongShaderGL::drawInstanced(Mesh const& mesh, unsigned amount)
{
}

const vec3& PhongShaderGL::getLightColor() const
{
	return lightColor;
}

const vec3& PhongShaderGL::getLightPosition() const
{
	return lightPosition;
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

void PhongShaderGL::setMaterial(const PhongMaterial& material)
{
	this->material = &material;
}

void PhongShaderGL::use()
{
	glUseProgram(this->programID);
}
