#include <shader/opengl/DepthMapShaderGL.hpp>
#include <glm/glm.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

CubeDepthMapShaderGL::CubeDepthMapShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	ShaderGL(vertexShaderFile, fragmentShaderFile), cubeMap(nullptr)
{
}

CubeDepthMapShaderGL::~CubeDepthMapShaderGL()
{
}

void CubeDepthMapShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	use();
	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(projection * view));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->getCubeMap());
	glUniform1i(glGetUniformLocation(programID, "cubeMap"), 0);

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void CubeDepthMapShaderGL::drawInstanced(Mesh const& mesh, unsigned amount)
{
}

void CubeDepthMapShaderGL::release()
{
	ShaderGL::release();
}

void CubeDepthMapShaderGL::use()
{
	ShaderGL::use();
}

void CubeDepthMapShaderGL::useCubeDepthMap(CubeMap* map)
{
	this->cubeMap = dynamic_cast<CubeMapGL*>(map);
	assert(this->cubeMap != nullptr);
}

DepthMapShaderGL::DepthMapShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	ShaderGL(vertexShaderFile, fragmentShaderFile), DepthMapShader(), texture(nullptr)
{
}

DepthMapShaderGL::~DepthMapShaderGL()
{
}

void DepthMapShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	use();
	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(projection * view));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->getTexture());
	glUniform1i(glGetUniformLocation(getProgramID(), "depthMap"), 0);

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void DepthMapShaderGL::drawInstanced(Mesh const& mesh, unsigned amount)
{
}

void DepthMapShaderGL::release()
{
	ShaderGL::release();
}

void DepthMapShaderGL::use()
{
	ShaderGL::use();
}

void DepthMapShaderGL::useDepthMapTexture(Texture* texture)
{
	this->texture = dynamic_cast<TextureGL*>(texture);
	assert(this->texture != nullptr);
}