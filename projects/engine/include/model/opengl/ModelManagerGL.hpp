#ifndef ENGINE_MODEL_OPENGL_MODELMANAGERGL_HPP
#define ENGINE_MODEL_OPENGL_MODELMANAGERGL_HPP
#include <memory>
#include <unordered_map>
#include <model/opengl/ModelGL.hpp>
#include <model/ModelManager.hpp>
#include <model/opengl/AssimpModelLoader.hpp>

class MeshGL;

class ModelManagerGL : public ModelManager
{
public:
	~ModelManagerGL() override;

	/*
	 * \param xPos : The x position of the sprite model measured in screen space.
	 * \param yPos : The y position of the sprite model measured in screen space.
	 * \param widthWeight : specifies the width of the model as a percentage of the active viewport width.
	 *		A value of 1.0f means full viewport width, 0.0f means no width analogously. 
	 * \param heightWeight : specifies the height of the model as a percentage of the active viewport height. 
	 */
	static ModelGL createSpriteModel(float xPos, float yPos, float widthWeight, float heightWeight);
	Model* getModel(const std::string& meshName) override;
	Model* getPositionNormalTexCube() override;
	static ModelManagerGL* get();

	void loadModels() override;

private:
	ModelManagerGL();
	
	static std::unique_ptr<ModelManagerGL> instance;

	std::unordered_map<std::string, std::shared_ptr<ModelGL>> models;
	AssimpModelLoader assimpLoader;
};
#endif