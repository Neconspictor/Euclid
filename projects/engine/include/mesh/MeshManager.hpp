#ifndef MODEL_FACTORY
#define MODEL_FACTORY

#include <mesh/Mesh.hpp>

/**
 * A mesh manager provides a central access point for creating and receiving 
 * 3d meshes. All memory allocated by a mesh is held by the manager and no user
 * memory management is needed. If the mesh manager isn't needed anymore, the 
 * allocated memory can be released by calling the manager's release method.
 */
class MeshManager
{
public:
	MeshManager(){};
	virtual ~MeshManager(){}

	/**
	 * Provides read access to a cube that has a vertex format that can be used with a SimpleLightShader.
	 */
	virtual Mesh* getSimpleLitCube() = 0;

	/**
	 * Provides a model that has a vertex format that allows to bind a 2D texture to it. 
	 */
	virtual Mesh* getTexturedCube() = 0;

	/**
	 * Provides access to a mesh by its name.
	 * NOTE: If the specfied mesh cannot be found, a MeshNotFoundException is thrown.
	 */
	virtual Mesh* getMesh(const std::string&  meshName) = 0;

	/**
	 * loads all meshes
	 */
	virtual void loadMeshes() = 0;
};
#endif