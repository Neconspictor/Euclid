#include <shader/opengl/SimpleReflectionShaderGL.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace glm;

SimpleReflectionShaderGL::SimpleReflectionShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	ShaderGL(vertexShaderFile, fragmentShaderFile), SimpleReflectionShader(), reflectionTexture(nullptr), cameraPosition(0,0,0)
{
}

SimpleReflectionShaderGL::~SimpleReflectionShaderGL()
{
}

void SimpleReflectionShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	use();

	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	mat4 transform = projection * view * model;
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transform));

	GLuint modelLoc = glGetUniformLocation(getProgramID(), "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

	GLint normalMatrixLoc = glGetUniformLocation(getProgramID(), "normalMatrix");
	mat4 normalMatrix = transpose(inverse(model));
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, value_ptr(normalMatrix));

	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, reflectionTexture->getCubeMap());
	glUniform1i(glGetUniformLocation(getProgramID(), "reflectionTexture"), 0);

	glUniform3f(glGetUniformLocation(programID, "cameraPos"), 
		cameraPosition.x, cameraPosition.y, cameraPosition.z);

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void SimpleReflectionShaderGL::drawInstanced(Mesh const& mesh, unsigned amount)
{
}

void SimpleReflectionShaderGL::release()
{
	ShaderGL::release();
}

void SimpleReflectionShaderGL::setCameraPosition(vec3 position)
{
	cameraPosition = std::move(position);
}

void SimpleReflectionShaderGL::setReflectionTexture(CubeMap* reflectionTex)
{
	reflectionTexture = static_cast<CubeMapGL*>(reflectionTex);
}

void SimpleReflectionShaderGL::use()
{
	ShaderGL::use();
}