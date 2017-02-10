#include <shader/opengl/SimpleColorShaderGL.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace std;
using namespace glm;

SimpleColorShaderGL::SimpleColorShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile)
	: ShaderGL(vertexShaderFile, fragmentShaderFile), objectColor(1, 1, 1, 1)
{
}

SimpleColorShaderGL::~SimpleColorShaderGL()
{
}

void SimpleColorShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	use();
	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(projection * view * model));

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void SimpleColorShaderGL::drawInstanced(Mesh const& mesh, unsigned amount)
{
}

const vec4& SimpleColorShaderGL::getObjectColor() const
{
	return objectColor;
}

void SimpleColorShaderGL::release()
{
	ShaderGL::release();
}

void SimpleColorShaderGL::setObjectColor(vec4 color)
{
	objectColor = move(color);
}

void SimpleColorShaderGL::use()
{
	glUseProgram(this->programID);
	GLint objectColorLoc = glGetUniformLocation(getProgramID(), "objectColor");
	glUniform4f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z, objectColor.w);
}