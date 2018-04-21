#pragma once

#include <model/Model.hpp>

/**
 * A mesh manager provides a central access point for creating and receiving 
 * 3d meshes. All memory allocated by a mesh is held by the manager and no user
 * memory management is needed. If the mesh manager isn't needed anymore, the 
 * allocated memory can be released by calling the manager's release method.
 */
class ModelManager
{
public:

	static const char* SKYBOX_MODEL_NAME;
	static const char* SPRITE_MODEL_NAME;

	ModelManager(){};
	virtual ~ModelManager(){}

	virtual Model* getSkyBox() = 0;

	virtual Model* getSprite() = 0;
	/*
	 * Provides read acces to a cube that has position, normal and texture coordinates.
	 */
	virtual Model* getPositionNormalTexCube() = 0;

	/**
	 * Provides access to a mesh by its name.
	 * NOTE: If the specfied mesh cannot be found, a MeshNotFoundException is thrown.
	 */
	virtual Model* getModel(const std::string&  meshName, Shaders materialShader) = 0;

	/**
	 * loads all meshes
	 */
	virtual void loadModels() = 0;

	virtual void useInstances(Model* model, glm::mat4* modelMatrices, unsigned int amount) = 0;
};