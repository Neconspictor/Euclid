#pragma once
#include <model/Model.hpp>
#include <mesh/MeshGL.hpp>
#include <vector>

class ModelGL : public Model
{
public:
	
	ModelGL(std::vector<std::unique_ptr<MeshGL>> meshes);
	ModelGL(const ModelGL&) = delete;
	ModelGL(ModelGL&& o);

	ModelGL& operator=(const ModelGL& o) = delete;
	ModelGL& operator=(ModelGL&& o);
	virtual ~ModelGL() override;

	void createInstanced(unsigned instanceAmount, glm::mat4* modelMatrices);

	bool instancedUsed() const;

	void setInstanced(bool value);

protected:
	std::vector<std::unique_ptr<MeshGL>> meshes;
	bool instanced;
	GLuint matrixBuffer;

private:
	static std::vector<std::reference_wrapper<Mesh>> createReferences(const std::vector<std::unique_ptr<MeshGL>>& meshes);
};