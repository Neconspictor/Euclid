#ifndef ENGINE_MODEL_OPENGL_MODELMANAGERGL_HPP
#define ENGINE_MODEL_OPENGL_MODELMANAGERGL_HPP
#include <mesh/MeshManager.hpp>
#include <memory>
#include <unordered_map>

class MeshGL;

class MeshManagerGL : public MeshManager
{
public:
	~MeshManagerGL() override;
	Mesh* getMesh(const std::string& meshName) override;
	Mesh* getSimpleLitCube() override;
	Mesh* getTexturedCube() override;

	static MeshManagerGL* get();

	void loadMeshes() override;

private:
	MeshManagerGL();
	static void meshRelease(MeshGL* mesh);
	
	static std::unique_ptr<MeshManagerGL> instance;

	std::unordered_map<std::string, std::shared_ptr<MeshGL>> meshes;
};
#endif