#pragma once
#include <nex/opengl/mesh/MeshGL.hpp>
#include <vector>
#include <memory>


namespace nex
{
	class ShaderProgram;

	class ModelGL
	{
	public:

		ModelGL(std::vector<std::unique_ptr<MeshGL>> meshes, std::vector<std::unique_ptr<Material>> materials);

		ModelGL(const ModelGL&) = delete;
		ModelGL& operator=(const ModelGL& o) = delete;

		//void createInstanced(unsigned instanceAmount, glm::mat4* modelMatrices);

		bool instancedUsed() const;

		void setInstanced(bool value);

		const std::vector<std::unique_ptr<MeshGL>>& getMeshes() const;
		const std::vector<std::unique_ptr<Material>>& getMaterials() const;

		void draw(ShaderProgram* shader) {};

	protected:
		std::vector<std::unique_ptr<MeshGL>> mMeshes;
		std::vector<std::unique_ptr<Material>> mMaterials;
		bool instanced;
	};
}