#ifndef ENGINE_MODEL_OPENGL_MODELGL_HPP
#define ENGINE_MODEL_OPENGL_MODELGL_HPP
#include <model/Model.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <vector>

class ModelGL : public Model
{
public:
	explicit ModelGL(std::vector<MeshGL> meshes);
	ModelGL(const ModelGL& o);
	ModelGL(ModelGL&& o);
	ModelGL& operator=(const ModelGL& o);
	ModelGL& operator=(ModelGL&& o);

protected:
	std::vector<MeshGL> glMeshes;

	void updateMeshPointers();
};

#endif