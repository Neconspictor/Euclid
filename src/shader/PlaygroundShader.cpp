#include <shader/PlaygroundShader.hpp>
#include <texture/TextureManager.hpp>
#include <glm/gtc/type_ptr.hpp>

PlaygroundShader::PlaygroundShader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	Shader(vertexShaderFile, fragmentShaderFile), texture(GL_FALSE), texture2(GL_FALSE), mixValue(0)
{
}

PlaygroundShader::~PlaygroundShader()
{
}

void PlaygroundShader::setTexture1(const std::string& textureName)
{
	texture = TextureManager::getInstance()->loadImage(textureName);
}

void PlaygroundShader::setTexture2(const std::string& textureName)
{
	texture2 = TextureManager::getInstance()->loadImage(textureName);
}

void PlaygroundShader::setTextureMixValue(float mixValue)
{
	this->mixValue = mixValue;
}

void PlaygroundShader::release()
{
	Shader::release();
}

void PlaygroundShader::use()
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

void PlaygroundShader::draw(Model const& model, glm::mat4 const& transform)
{
	use();
	glBindVertexArray(model.getVertexArrayObject());
	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transform));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, model.getVertexCount());
	glBindVertexArray(0);
}