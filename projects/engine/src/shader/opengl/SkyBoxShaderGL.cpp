#include <shader/opengl/SkyBoxShaderGL.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace glm;

SkyBoxShaderGL::SkyBoxShaderGL(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) : 
	SkyBoxShader(), skyTexture(nullptr), ShaderGL(vertexShaderFile, fragmentShaderFile)
{
}

SkyBoxShaderGL::~SkyBoxShaderGL()
{
}

void SkyBoxShaderGL::draw(Mesh const& meshOriginal)
{
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	use();

	GLuint transformLoc = glGetUniformLocation(getProgramID(), "transform");
	mat4 transform = projection * view;
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transform));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture->getCubeMap());
	glUniform1i(glGetUniformLocation(getProgramID(), "skybox"), 0);

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDepthFunc(GL_LESS); // The Type Of Depth Testing To Do
}

void SkyBoxShaderGL::release()
{
	ShaderGL::release();
}

void SkyBoxShaderGL::setSkyTexture(CubeMap* sky)
{
	skyTexture = static_cast<CubeMapGL*>(sky);
}

void SkyBoxShaderGL::use()
{
	ShaderGL::use();
}
