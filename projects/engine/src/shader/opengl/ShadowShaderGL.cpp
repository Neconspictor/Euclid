#include <shader/opengl/ShadowShaderGL.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace std;
using namespace glm;

ShadowShaderGL::ShadowShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile)
	: ShaderGL(vertexShaderFile, fragmentShaderFile)
{
}

ShadowShaderGL::~ShadowShaderGL()
{
}

void ShadowShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	use();
	GLuint lightSpaceMatrixLoc = glGetUniformLocation(getProgramID(), "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, value_ptr(projection * view));

	GLuint modelLoc = glGetUniformLocation(getProgramID(), "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void ShadowShaderGL::drawInstanced(Mesh const& mesh, unsigned amount)
{
}

void ShadowShaderGL::release()
{
	ShaderGL::release();
}

void ShadowShaderGL::use()
{
	glUseProgram(this->programID);
}