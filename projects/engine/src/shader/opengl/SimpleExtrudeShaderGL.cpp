#include <shader/opengl/SimpleExtrudeShaderGL.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace std;
using namespace glm;

SimpleExtrudeShaderGL::SimpleExtrudeShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile)
	: ShaderGL(vertexShaderFile, fragmentShaderFile), SimpleExtrudeShader(), objectColor(1, 1, 1, 1), extrudeValue(0)
{
}

SimpleExtrudeShaderGL::~SimpleExtrudeShaderGL()
{
}

void SimpleExtrudeShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);

	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	use();
	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(projection * view * model));

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndices().size());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

const vec4& SimpleExtrudeShaderGL::getObjectColor() const
{
	return objectColor;
}

void SimpleExtrudeShaderGL::release()
{
	ShaderGL::release();
}

void SimpleExtrudeShaderGL::setExtrudeValue(float extrudeValue)
{
	this->extrudeValue = extrudeValue;
}

void SimpleExtrudeShaderGL::setObjectColor(vec4 color)
{
	objectColor = move(color);
}

void SimpleExtrudeShaderGL::use()
{
	glUseProgram(this->programID);
	GLint objectColorLoc = glGetUniformLocation(getProgramID(), "objectColor");
	GLint extrudeValueLoc = glGetUniformLocation(getProgramID(), "extrudeValue");
	glUniform4f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z, objectColor.w);
	glUniform1f(extrudeValueLoc, extrudeValue);
}