#include <model/Model.hpp>

using namespace std;
using namespace glm;

Model::Model(string meshName, mat4 mat)
{
	this->meshName = move(meshName);
	trafo = move(mat);
}

string const& Model::getMeshName() const
{
	return meshName;
}

mat4 const& Model::getTrafo() const
{
	return trafo;
}

void Model::setTrafo(mat4 mat)
{
	trafo = move(mat);
}