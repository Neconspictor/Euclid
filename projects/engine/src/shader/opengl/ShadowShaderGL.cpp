#include <shader/opengl/ShadowShaderGL.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>

using namespace std;
using namespace glm;

PointShadowShaderGL::PointShadowShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile,
	const string& geometryShaderFile) : ShaderGL(vertexShaderFile, fragmentShaderFile,
	geometryShaderFile), PointShadowShader(), matrices(nullptr), range(0)
{
}

PointShadowShaderGL::~PointShadowShaderGL()
{
}

void PointShadowShaderGL::draw(Mesh const& meshOriginal)
{
	assert(matrices != nullptr);
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	use();
	GLuint lightSpaceMatrixLoc = glGetUniformLocation(getProgramID(), "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, value_ptr(projection * view));

	GLuint modelLoc = glGetUniformLocation(getProgramID(), "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

	for (GLuint i = 0; i < 6; ++i)
	{
		string matrixDesc = "shadowMatrices[" + to_string(i) + "]";
		GLuint loc = glGetUniformLocation(getProgramID(), matrixDesc.c_str());
		glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(matrices[i]));
	}

	GLuint rangeLoc = glGetUniformLocation(getProgramID(), "range");
	glUniform1f(rangeLoc, range);
	
	GLuint lightPosLoc = glGetUniformLocation(getProgramID(), "lightPos");
	glUniform3fv(lightPosLoc, 1, value_ptr(lightPos));

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void PointShadowShaderGL::drawInstanced(Mesh const& mesh, unsigned amount)
{
}

void PointShadowShaderGL::release()
{
	ShaderGL::release();
}

void PointShadowShaderGL::setLightPosition(vec3 pos)
{
	this->lightPos = move(pos);
}

void PointShadowShaderGL::setRange(float range)
{
	this->range = range;
}

void PointShadowShaderGL::setShadowMatrices(mat4 matrices[6])
{
	this->matrices = matrices;
}

void PointShadowShaderGL::use()
{
	ShaderGL::use();
}

ShadowShaderGL::ShadowShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile)
	: ShaderGL(vertexShaderFile, fragmentShaderFile)
{
}

ShadowShaderGL::~ShadowShaderGL()
{
}

void ShadowShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;
	use();
	GLuint lightSpaceMatrixLoc = glGetUniformLocation(getProgramID(), "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, value_ptr(projection * view));

	GLuint modelLoc = glGetUniformLocation(getProgramID(), "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void ShadowShaderGL::drawInstanced(Mesh const& mesh, unsigned amount)
{
}

void ShadowShaderGL::release()
{
	ShaderGL::release();
}

void ShadowShaderGL::use()
{
	glUseProgram(this->programID);
}