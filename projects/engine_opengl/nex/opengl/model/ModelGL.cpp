#include <nex/opengl/model/ModelGL.hpp>

using namespace std;
using namespace glm;

ModelGL::ModelGL(vector<unique_ptr<MeshGL>> meshes)
{
	this->meshes = move(meshes);
	this->meshReferences = move(createReferences(meshes));
}

ModelGL::ModelGL(ModelGL&& o) noexcept : meshes(o.meshes), 
meshReferences(move(o.meshReferences)), 
instanced(o.instanced), 
vertexAttributeBuffer(o.vertexAttributeBuffer)
{
	o.meshes.clear();
	o.meshReferences.clear();

	o.vertexAttributeBuffer = GL_FALSE;

}

ModelGL& ModelGL::operator=(ModelGL&& o) noexcept
{
	if (this == &o) return *this;
	this->meshes = move(o.meshes);
	o.meshes.clear();
	return *this;
}

ModelGL::~ModelGL()
{
}

void ModelGL::createInstanced(unsigned amount, mat4* modelMatrices)
{
	// Vertex Buffer Object
	glGenBuffers(1, &vertexAttributeBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexAttributeBuffer);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(mat4), &modelMatrices[0], GL_STATIC_DRAW);

	for (GLuint i = 0; i < meshes.size(); i++)
	{
		MeshGL& mesh = *meshes[i];
		GLuint VAO = mesh.getVertexArrayObject();
		glBindVertexArray(VAO);

		// Vertex Attributes
		size_t vec4Size = sizeof(vec4);
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, vertexAttributeBuffer);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (GLvoid*)0);
		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, vertexAttributeBuffer);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (GLvoid*)(vec4Size));
		glEnableVertexAttribArray(5);
		glBindBuffer(GL_ARRAY_BUFFER, vertexAttributeBuffer);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (GLvoid*)(2 * vec4Size));
		glEnableVertexAttribArray(6);
		glBindBuffer(GL_ARRAY_BUFFER, vertexAttributeBuffer);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (GLvoid*)(3 * vec4Size));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
}

bool ModelGL::instancedUsed()const
{
	return instanced;
}

void ModelGL::setInstanced(bool value)
{
	instanced = value;
}

void ModelGL::draw(ShaderGL* shader)
{
}

const vector<std::reference_wrapper<MeshGL>>& ModelGL::getMeshes() const
{
	return meshReferences;
}

std::vector<std::reference_wrapper<MeshGL>> ModelGL::createReferences(const std::vector<std::unique_ptr<MeshGL>>& meshes)
{
	std::vector<std::reference_wrapper<MeshGL>> result;
	for (auto&& elem : meshes) {
		result.push_back(*elem.get());
	}
	return result;
}