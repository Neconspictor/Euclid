#include <shader/opengl/ScreenShaderGL.hpp>
#include <glm/glm.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace std;
using namespace glm;

ScreenShaderGL::ScreenShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile) :
	ScreenShader(), ShaderGL(vertexShaderFile, fragmentShaderFile), frameBuffer(GL_FALSE)
{
}

ScreenShaderGL::~ScreenShaderGL()
{
}

void ScreenShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, frameBuffer);
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

void ScreenShaderGL::setOffscreenBuffer(GLuint frameBuffer)
{
	this->frameBuffer = frameBuffer;
}

void ScreenShaderGL::use()
{
	ShaderGL::use();
}
