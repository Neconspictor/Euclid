#ifndef MODEL_FACTORY
#define MODEL_FACTORY

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
	ModelManager(){};
	virtual ~ModelManager(){}

	/**
	 * Provides read access to a cube that has only position data.
	 */
	virtual Model* getPositionCube() = 0;

	/**
	* Provides read access to a cube that has position and vertex normal data.
	*/
	virtual Model* getPositionNormalCube() = 0;

	/*
	 * Provides read acces to a cube that has position, normal and texture coordinates.
	 */
	virtual Model* getPositionNormalTexCube() = 0;

	/**
	 * Provides a model that has a vertex format that contains position and uv coordinates.
	 */
	virtual Model* getTexturedCube() = 0;

	/**
	 * Provides access to a mesh by its name.
	 * NOTE: If the specfied mesh cannot be found, a MeshNotFoundException is thrown.
	 */
	virtual Model* getModel(const std::string&  meshName) = 0;

	/**
	 * loads all meshes
	 */
	virtual void loadModels() = 0;
};
#endif