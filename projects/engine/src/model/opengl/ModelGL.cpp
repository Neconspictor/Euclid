#include <model/opengl/ModelGL.hpp>

using namespace std;

ModelGL::ModelGL(vector<MeshGL> meshes) : Model({})
{
	this->glMeshes = meshes;
	this->meshes = vector<Mesh*>();
	for (auto mesh : glMeshes)
	{
		this->meshes.push_back(static_cast<Mesh*>(&mesh));
	}
}

ModelGL::ModelGL(const ModelGL& o) : Model(o), glMeshes(o.glMeshes)
{
}

ModelGL::ModelGL(ModelGL&& o) : Model(o), glMeshes(o.glMeshes)
{
}

ModelGL& ModelGL::operator=(const ModelGL& o)
{
	if (this == &o) return *this;
	meshes = vector<Mesh*>();
	glMeshes = o.glMeshes;
	for (auto mesh : glMeshes)
	{
		meshes.push_back(static_cast<Mesh*>(&mesh));
	}

	return *this;
}

ModelGL& ModelGL::operator=(ModelGL&& o)
{
	if (this == &o) return *this;
	meshes = move(o.meshes);
	glMeshes = move(o.glMeshes);
	o.meshes.clear();
	o.glMeshes.clear();
	return *this;
}

vector<Mesh*> ModelGL::getMeshes() const
{
	vector<Mesh*> result;
	for (int i = 0; i < glMeshes.size(); ++i)
	{
		result.push_back((Mesh*)(&glMeshes[i]));
	}

	return result;
}
