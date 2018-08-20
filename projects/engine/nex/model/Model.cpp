#include <nex/model/Model.hpp>

using namespace std;

Model::Model(vector<std::reference_wrapper<Mesh>> meshReferences)
{
	this->meshReferences = meshReferences;
}


Model::Model(Model&& o) : meshReferences(move(o.meshReferences))
{
}

Model& Model::operator=(Model&& o)
{
	if (this == &o) return *this;
	meshReferences = move(o.meshReferences);
	return *this;
}

Model::~Model()
{
}

void Model::draw(Shader* shader)
{
}

const vector<std::reference_wrapper<Mesh>>& Model::getMeshes() const
{
	return meshReferences;
}