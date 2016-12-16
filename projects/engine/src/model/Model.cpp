#include <model/Model.hpp>

using namespace std;

Model::Model(vector<Mesh*> meshes)
{
	this->meshes = move(meshes);
}

Model::Model(const Model& o) : meshes(o.meshes)
{
}

Model::Model(Model&& o) : meshes(o.meshes)
{
}

Model& Model::operator=(const Model& o)
{
	if (this == &o) return *this;
	meshes = o.meshes;
	return *this;
}

Model& Model::operator=(Model&& o)
{
	if (this == &o) return *this;
	meshes = move(o.meshes);
	return *this;
}

Model::~Model()
{
}

void Model::draw(Shader* shader)
{
}

vector<Mesh*> Model::getMeshes() const
{
	return meshes;
}
