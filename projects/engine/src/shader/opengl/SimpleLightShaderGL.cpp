#include <shader/opengl/SimpleLightShaderGL.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <exception/MeshNotFoundException.hpp>

SimpleLightShaderGL::SimpleLightShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile)
	: ShaderGL(vertexShaderFile, fragmentShaderFile), lightColor(1, 1, 1), objectColor(1, 1, 1)
{
}

SimpleLightShaderGL::~SimpleLightShaderGL()
{
}

void SimpleLightShaderGL::draw(Model const& model, glm::mat4 const& transform)
{
	MeshGL* mesh = getFromModel(model);
	use();
	glBindVertexArray(mesh->getVertexArrayObject());
	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transform));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount());
	glBindVertexArray(0);
}

glm::vec3 SimpleLightShaderGL::getLightColor()
{
	return lightColor;
}

glm::vec3 SimpleLightShaderGL::getObjectColor()
{
	return objectColor;
}

bool SimpleLightShaderGL::loadingFailed()
{
	return ShaderGL::loadingFailed();
}

void SimpleLightShaderGL::release()
{
	ShaderGL::release();
}

void SimpleLightShaderGL::setLightColor(glm::vec3 color)
{
	lightColor = color;
}

void SimpleLightShaderGL::setObjectColor(glm::vec3 color)
{
	objectColor = color;
}

void SimpleLightShaderGL::use()
{
	glUseProgram(this->programID);
	GLint objectColorLoc = glGetUniformLocation(getProgramID(), "objectColor");
	GLint lightColorLoc = glGetUniformLocation(getProgramID(), "lightColor");
	glUniform3f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z);
}