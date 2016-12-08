#include <model/opengl/ModelGL.hpp>

using namespace std;

ModelGL::ModelGL(vector<MeshGL> meshes) : Model({})
{
	this->glMeshes = meshes;
	updateMeshPointers();
}

ModelGL::ModelGL(const ModelGL& o) : Model(o), glMeshes(o.glMeshes)
{
	updateMeshPointers();
}

ModelGL::ModelGL(ModelGL&& o) : Model(o), glMeshes(o.glMeshes)
{
	updateMeshPointers();
}

ModelGL& ModelGL::operator=(const ModelGL& o)
{
	if (this == &o) return *this;
	meshes = vector<Mesh*>();
	glMeshes = o.glMeshes;
	updateMeshPointers();

	return *this;
}

ModelGL& ModelGL::operator=(ModelGL&& o)
{
	if (this == &o) return *this;
	meshes = move(o.meshes);
	glMeshes = move(o.glMeshes);
	o.meshes.clear();
	o.glMeshes.clear();
	updateMeshPointers();
	return *this;
}

void ModelGL::updateMeshPointers()
{
	this->meshes.clear();
	for (int i = 0; i < glMeshes.size(); ++i)
	{
		this->meshes.push_back((Mesh*)(&glMeshes[i]));
	}
}