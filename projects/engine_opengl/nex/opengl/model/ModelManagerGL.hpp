#pragma once
#include <vector>
#include <unordered_map>
#include <nex/opengl/model/ModelGL.hpp>
#include <nex/model/ModelManager.hpp>
#include <nex/opengl/model/AssimpModelLoader.hpp>
#include <nex/material/PbrMaterialLoader.hpp>
#include <nex/material/BlinnPhongMaterialLoader.hpp>
#include <memory>

class MeshGL;

class ModelManagerGL : public ModelManager
{
public:

	ModelManagerGL();

	virtual ~ModelManagerGL() override;

	static ModelManagerGL* get();

	virtual Model* getSkyBox() override;

	Model* getModel(const std::string& meshName, Shaders materialShader) override;
	Model* getPositionNormalTexCube() override;

	/*
	* \param xPos : The x position of the sprite model measured in screen space.
	* \param yPos : The y position of the sprite model measured in screen space.
	* \param widthWeight : specifies the width of the model as a percentage of the active viewport width.
	*		A value of 1.0f means full viewport width, 0.0f means no width analogously.
	* \param heightWeight : specifies the height of the model as a percentage of the active viewport height.
	*/
	Model* getSprite() override;

	void loadModels() override;

	virtual void useInstances(Model* model, glm::mat4* modelMatrices, unsigned int amount) override;

private:

	ModelManagerGL(const ModelManagerGL&) = delete;
	ModelManagerGL& operator=(const ModelManagerGL&) = delete;

	
	static std::unique_ptr<ModelManagerGL> instance;
	std::vector<std::unique_ptr<ModelGL>> models;
	std::unordered_map<std::string, ModelGL*> modelTable;
	AssimpModelLoader assimpLoader;
	PbrMaterialLoader pbrMaterialLoader;
	BlinnPhongMaterialLoader blinnPhongMaterialLoader;
};