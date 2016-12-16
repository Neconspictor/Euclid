#include <shader/opengl/PlaygroundShaderGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <texture/opengl/TextureManagerGL.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace glm;

PlaygroundShaderGL::PlaygroundShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	ShaderGL(vertexShaderFile, fragmentShaderFile), PlaygroundShader(), mixValue(0), texture(nullptr), texture2(nullptr)
{
}

PlaygroundShaderGL::~PlaygroundShaderGL()
{
}


void PlaygroundShaderGL::draw(Mesh const& meshOriginal)
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
	glUseProgram(0);
}

void PlaygroundShaderGL::release()
{
	ShaderGL::release();
}

void PlaygroundShaderGL::setTexture1(const std::string& textureName)
{
	texture = TextureManagerGL::get()->getImageGL(textureName);
}

void PlaygroundShaderGL::setTexture2(const std::string& textureName)
{
	texture2 = TextureManagerGL::get()->getImageGL(textureName);
}

void PlaygroundShaderGL::setTextureMixValue(float mixValue)
{
	this->mixValue = mixValue;
}

void PlaygroundShaderGL::use()
{
	glUseProgram(programID);
	glActiveTexture(GL_TEXTURE0);	// Activate the texture unit first before binding texture
	glBindTexture(GL_TEXTURE_2D, texture->getTexture());
	glUniform1i(glGetUniformLocation(getProgramID(), "diffuse"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2->getTexture());
	glUniform1i(glGetUniformLocation(getProgramID(), "diffuseMultiply"), 1);
	glUniform1f(glGetUniformLocation(getProgramID(), "mixValue"), mixValue);
}