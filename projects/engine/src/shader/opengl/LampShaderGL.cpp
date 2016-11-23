#include <shader/opengl/LampShaderGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <model/Model.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace std;

LampShaderGL::LampShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	LampShader(), ShaderGL(vertexShaderFile, fragmentShaderFile)
{
}

LampShaderGL::~LampShaderGL()
{
}

void LampShaderGL::draw(Model const& model, glm::mat4 const& transform)
{
	MeshGL* mesh = getFromModel(model);
	use();
	glBindVertexArray(mesh->getVertexArrayObject());

	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transform));

	glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount());
	glBindVertexArray(0);
}

bool LampShaderGL::loadingFailed()
{
	return ShaderGL::loadingFailed();
}

void LampShaderGL::release()
{
	ShaderGL::release();
}

void LampShaderGL::use()
{
	ShaderGL::use();
}