#pragma once
#include <nex/mesh/SubMesh.hpp>
#include <vector>
#include <memory>


namespace nex
{
	class Shader;

	class StaticMesh
	{
	public:

		StaticMesh(std::vector<std::unique_ptr<SubMesh>> meshes, std::vector<std::unique_ptr<Material>> materials);

		StaticMesh(const StaticMesh&) = delete;
		StaticMesh& operator=(const StaticMesh& o) = delete;

		//void createInstanced(unsigned instanceAmount, glm::mat4* modelMatrices);

		bool instancedUsed() const;

		void setInstanced(bool value);

		const std::vector<std::unique_ptr<SubMesh>>& getMeshes() const;
		const std::vector<std::unique_ptr<Material>>& getMaterials() const;

		void draw(Shader* shader) {};

	protected:
		std::vector<std::unique_ptr<SubMesh>> mMeshes;
		std::vector<std::unique_ptr<Material>> mMaterials;
		bool instanced;
	};
}