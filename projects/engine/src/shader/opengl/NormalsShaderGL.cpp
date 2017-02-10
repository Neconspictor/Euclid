#include <shader/opengl/NormalsShaderGL.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

NormalsShaderGL::NormalsShaderGL(const string& vertexShaderFile, 
	const string& fragmentShaderFile, const string& geometryShaderFile) :
	ShaderGL(vertexShaderFile, fragmentShaderFile, geometryShaderFile), color(0,0,0,1)
{
}

NormalsShaderGL::~NormalsShaderGL()
{
}

void NormalsShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = static_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	use();

	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(projection * view * model));

	GLint normalMatrixLoc = glGetUniformLocation(getProgramID(), "normalMatrix");
	mat3 normalMatrix = transpose(inverse(view * model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, value_ptr(normalMatrix));

	GLint projectionLoc = glGetUniformLocation(getProgramID(), "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));

	GLint colorLoc = glGetUniformLocation(getProgramID(), "color");
	glUniform4f(colorLoc, color.r, color.g, color.b, color.a);

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void NormalsShaderGL::drawInstanced(Mesh const& mesh, unsigned amount)
{
}

const vec4& NormalsShaderGL::getNormalColor() const
{
	return color;
}

void NormalsShaderGL::release()
{
	ShaderGL::release();
}

void NormalsShaderGL::setNormalColor(vec4 color)
{
	this->color = move(color);
}

void NormalsShaderGL::use()
{
	ShaderGL::use();
}