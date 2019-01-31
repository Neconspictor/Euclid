#include <nex/opengl/model/ModelGL.hpp>
#include "nex/opengl/renderer/RendererOpenGL.hpp"

using namespace std;
using namespace glm;

namespace nex
{
	ModelGL::ModelGL(vector<unique_ptr<MeshGL>> meshes, std::vector<std::unique_ptr<Material>> materials) :
		mMeshes(std::move(meshes)), mMaterials(std::move(materials)), instanced(false)
	{
	}

	// TODO code has to be updated for new MeshGL class
	/*void ModelGL::createInstanced(unsigned amount, mat4* modelMatrices)
	{
		// Vertex Buffer Object
		GLCall(glGenBuffers(1, &vertexAttributeBuffer));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexAttributeBuffer));
		GLCall(glBufferData(GL_ARRAY_BUFFER, amount * sizeof(mat4), &modelMatrices[0], GL_STATIC_DRAW));

		for (GLuint i = 0; i < meshes.size(); i++)
		{
			MeshGL& mesh = *meshes[i];
			GLuint VAO = mesh.getVertexArrayObject();
			GLCall(glBindVertexArray(VAO));

			// Vertex Attributes
			size_t vec4Size = sizeof(vec4);
			GLCall(glEnableVertexAttribArray(3));
			GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexAttributeBuffer));
			GLCall(glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (GLvoid*)0));
			GLCall(glEnableVertexAttribArray(4));
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

			GLCall(glBindVertexArray(0));
		}
	}*/

	bool ModelGL::instancedUsed()const
	{
		return instanced;
	}

	void ModelGL::setInstanced(bool value)
	{
		instanced = value;
	}

	const std::vector<std::unique_ptr<MeshGL>>& ModelGL::getMeshes() const
	{
		return mMeshes;
	}

	const std::vector<std::unique_ptr<Material>>& ModelGL::getMaterials() const
	{
		return mMaterials;
	}
}