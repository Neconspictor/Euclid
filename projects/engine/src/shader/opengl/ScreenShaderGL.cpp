#include <shader/opengl/ScreenShaderGL.hpp>
#include <glm/glm.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <glm/gtc/type_ptr.inl>

using namespace std;
using namespace glm;

ScreenShaderGL::ScreenShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile) :
	ScreenShader(), ShaderGL(vertexShaderFile, fragmentShaderFile), texture(nullptr)
{
}

ScreenShaderGL::~ScreenShaderGL()
{
}

void ScreenShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	
	use();
	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(projection * view * model));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->getTexture());
	glUniform1i(glGetUniformLocation(getProgramID(), "screenTexture"), 0);

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void ScreenShaderGL::drawInstanced(Mesh const& mesh, unsigned amount)
{
}

void ScreenShaderGL::release()
{
	ShaderGL::release();
}

void ScreenShaderGL::use()
{
	ShaderGL::use();
}

void ScreenShaderGL::useTexture(Texture* texture)
{
	this->texture = dynamic_cast<TextureGL*>(texture);
	assert(this->texture);
}