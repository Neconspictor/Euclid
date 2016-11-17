#include <shader/opengl/PlaygroundShaderGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <renderer/RendererOpenGL.hpp>

using namespace glm;

PlaygroundShaderGL::PlaygroundShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	ShaderGL(vertexShaderFile, fragmentShaderFile), PlaygroundShader(), mixValue(0), texture(GL_FALSE), texture2(GL_FALSE)
{
}

PlaygroundShaderGL::~PlaygroundShaderGL()
{
}


void PlaygroundShaderGL::draw(Model const& model, mat4 const& transform)
{
	use();
	glBindVertexArray(model.getVertexArrayObject());
	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transform));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, model.getVertexCount());
	glBindVertexArray(0);
}

bool PlaygroundShaderGL::loadingFailed()
{
	return ShaderGL::loadingFailed();
}

void PlaygroundShaderGL::release()
{
	ShaderGL::release();
}

void PlaygroundShaderGL::setTexture1(const std::string& textureName)
{
	texture = RendererOpenGL::getTextureManagerGL()->getImage(textureName);
}

void PlaygroundShaderGL::setTexture2(const std::string& textureName)
{
	texture2 = RendererOpenGL::getTextureManagerGL()->getImage(textureName);
}

void PlaygroundShaderGL::setTextureMixValue(float mixValue)
{
	this->mixValue = mixValue;
}

void PlaygroundShaderGL::use()
{
	glUseProgram(programID);
	glActiveTexture(GL_TEXTURE0);	// Activate the texture unit first before binding texture
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(getProgramID(), "diffuse"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glUniform1i(glGetUniformLocation(getProgramID(), "diffuseMultiply"), 1);
	glUniform1f(glGetUniformLocation(getProgramID(), "mixValue"), mixValue);
}