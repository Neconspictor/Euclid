#include <shader/opengl/LampShaderGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <model/Vob.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace std;
using namespace glm;

LampShaderGL::LampShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile) :
	LampShader(), ShaderGL(vertexShaderFile, fragmentShaderFile)
{
}

LampShaderGL::~LampShaderGL()
{
}

void LampShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	use();

	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	mat4 transform = projection * view * model;
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transform));

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void LampShaderGL::release()
{
	ShaderGL::release();
}

void LampShaderGL::use()
{
	ShaderGL::use();
}