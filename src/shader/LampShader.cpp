#include <shader/LampShader.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

LampShader::LampShader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	Shader(vertexShaderFile, fragmentShaderFile)
{
}

LampShader::~LampShader()
{
}

void LampShader::draw(Model const& model, glm::mat4 const& transform)
{
	use();
	glBindVertexArray(model.getVertexArrayObject());
	
	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transform));
	
	glDrawArrays(GL_TRIANGLES, 0, model.getVertexCount());
	glBindVertexArray(0);
}