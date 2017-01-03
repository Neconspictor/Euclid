#pragma once
#include <model/Model.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <vector>

class ModelGL : public Model
{
public:
	explicit ModelGL(std::vector<MeshGL> meshes);
	ModelGL(const ModelGL& o);
	ModelGL(ModelGL&& o);
	ModelGL& operator=(const ModelGL& o);
	ModelGL& operator=(ModelGL&& o);

	void createInstanced(unsigned instanceAmount, glm::mat4* modelMatrices);

	const std::vector<MeshGL>& getGlMeshes();

	bool instancedUsed() const;

	void setInstanced(bool value);

protected:
	std::vector<MeshGL> glMeshes;
	bool instanced;
	GLuint matrixBuffer;

	void updateMeshPointers();
};