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

		ModelGL(std::vector<std::unique_ptr<MeshGL>> meshes);

		ModelGL(const ModelGL&) = delete;
		ModelGL& operator=(const ModelGL& o) = delete;

		virtual ~ModelGL() = default;

		//void createInstanced(unsigned instanceAmount, glm::mat4* modelMatrices);

		bool instancedUsed() const;

		void setInstanced(bool value);

		const std::vector<std::reference_wrapper<MeshGL>>& getMeshes() const;

		void draw(ShaderProgram* shader) {};

	protected:
		std::vector<std::reference_wrapper<MeshGL>> meshReferences;
		std::vector<std::unique_ptr<MeshGL>> meshes;
		bool instanced;
		GLuint vertexAttributeBuffer;

	private:
		static std::vector<std::reference_wrapper<MeshGL>> createReferences(const std::vector<std::unique_ptr<MeshGL>>& meshes);
	};
}