#pragma once
#include <nex/opengl/mesh/MeshGL.hpp>
#include <vector>
#include <memory>


class ShaderGL;

class ModelGL
{
public:
	
	ModelGL(std::vector<std::unique_ptr<MeshGL>> meshes);
	
	ModelGL(ModelGL&& o);
	ModelGL& operator=(ModelGL&& o);

	ModelGL(const ModelGL&) = delete;
	ModelGL& operator=(const ModelGL& o) = delete;
	
	virtual ~ModelGL();

	//void createInstanced(unsigned instanceAmount, glm::mat4* modelMatrices);

	bool instancedUsed() const;

	void setInstanced(bool value);

	const std::vector<std::reference_wrapper<MeshGL>>& getMeshes() const;

	void draw(ShaderGL* shader);

protected:
	std::vector<std::reference_wrapper<MeshGL>> meshReferences;
	std::vector<std::unique_ptr<MeshGL>> meshes;
	bool instanced;
	GLuint vertexAttributeBuffer;

private:
	static std::vector<std::reference_wrapper<MeshGL>> createReferences(const std::vector<std::unique_ptr<MeshGL>>& meshes);
};