#include "shader/SimpleLightShader.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

SimpleLightShader::SimpleLightShader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) : 
	Shader(vertexShaderFile, fragmentShaderFile), lightColor(1,1,1), objectColor(1,1,1)
{
}

SimpleLightShader::~SimpleLightShader()
{
}

vec3 SimpleLightShader::getObjectColor()
{
	return objectColor;
}

vec3 SimpleLightShader::getLightColor()
{
	return lightColor;
}

void SimpleLightShader::setObjectColor(vec3 color)
{
	objectColor = color;
}

void SimpleLightShader::release()
{
	Shader::release();
}

void SimpleLightShader::setLightColor(vec3 color)
{
	lightColor = color;
}

void SimpleLightShader::use()
{
	glUseProgram(this->programID);
	GLint objectColorLoc = glGetUniformLocation(getProgramID(), "objectColor");
	GLint lightColorLoc = glGetUniformLocation(getProgramID(), "lightColor");
	glUniform3f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z);
}

void SimpleLightShader::draw(Model const& model, mat4 const& trans)
{
	use();
	glBindVertexArray(model.getVertexArrayObject());
	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(trans));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, model.getVertexCount());
	glBindVertexArray(0);
}