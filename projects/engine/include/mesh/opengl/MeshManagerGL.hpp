#ifndef ENGINE_MODEL_OPENGL_MODELMANAGERGL_HPP
#define ENGINE_MODEL_OPENGL_MODELMANAGERGL_HPP
#include <mesh/MeshManager.hpp>
#include <memory>
#include <unordered_map>
#include <model/opengl/ModelGL.hpp>

class MeshGL;

class ModelManagerGL : public ModelManager
{
public:
	~ModelManagerGL() override;
	Model* getModel(const std::string& meshName) override;
	Model* getPositionCube() override;
	Model* getPositionNormalCube() override;
	Model* getPositionNormalTexCube() override;
	Model* getTexturedCube() override;

	static ModelManagerGL* get();

	void loadModels() override;

private:
	ModelManagerGL();
	
	static std::unique_ptr<ModelManagerGL> instance;

	std::unordered_map<std::string, std::shared_ptr<ModelGL>> models;
};
#endif