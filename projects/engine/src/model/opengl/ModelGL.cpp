#include <model/opengl/ModelGL.hpp>

using namespace std;
using namespace glm;

ModelGL::ModelGL(vector<MeshGL> meshes) : Model({})
{
	this->glMeshes = meshes;
	/*for (auto mesh : glMeshes)
	{
		meshes.push_back(mesh);
	}*/
	updateMeshPointers();
}

ModelGL::ModelGL(const ModelGL& o) : Model(o), glMeshes(o.glMeshes)
{
	updateMeshPointers();
}

ModelGL::ModelGL(ModelGL&& o) : Model(o), glMeshes(o.glMeshes)
{
	updateMeshPointers();
}

ModelGL& ModelGL::operator=(const ModelGL& o)
{
	if (this == &o) return *this;
	meshes = vector<Mesh*>();
	glMeshes = o.glMeshes;
	meshes = o.meshes;
	updateMeshPointers();

	return *this;
}

ModelGL& ModelGL::operator=(ModelGL&& o)
{
	if (this == &o) return *this;
	meshes = move(o.meshes);
	glMeshes = move(o.glMeshes);
	o.meshes.clear();
	o.glMeshes.clear();
	updateMeshPointers();
	return *this;
}

void ModelGL::createInstanced(unsigned amount, mat4* modelMatrices)
{
	// Vertex Buffer Object
	glGenBuffers(1, &matrixBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(mat4), &modelMatrices[0], GL_STATIC_DRAW);

	for (GLuint i = 0; i < glMeshes.size(); i++)
	{
		GLuint VAO = glMeshes[i].getVertexArrayObject();
		glBindVertexArray(VAO);

		// Vertex Attributes
		GLsizei vec4Size = sizeof(vec4);
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)0);
		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(vec4Size));
		glEnableVertexAttribArray(5);
		glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(2 * vec4Size));
		glEnableVertexAttribArray(6);
		glBindBuffer(GL_ARRAY_BUFFER, matrixBuffer);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(3 * vec4Size));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
}

const vector<MeshGL>& ModelGL::getGlMeshes()
{
	return glMeshes;
}

bool ModelGL::instancedUsed()const
{
	return instanced;
}

void ModelGL::setInstanced(bool value)
{
	instanced = value;
}

void ModelGL::updateMeshPointers()
{
	this->meshes.clear();
	for (int i = 0; i < glMeshes.size(); ++i)
	{
		this->meshes.push_back((Mesh*)(&glMeshes[i]));
	}
}
