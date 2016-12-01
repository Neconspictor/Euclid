#include <model/Model.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

Model::Model(string meshName)
{
	this->meshName = move(meshName);
	position = { 0,0,0 };
	scale = { 1,1,1 };
	orientation = { 1,0,0,0 };
}

void Model::calcTrafo()
{
	trafo = mat4();
	mat4 rotation = mat4_cast(orientation);
	mat4 scaleMat = glm::scale(trafo, scale);
	mat4 transMat = translate(trafo, position);
	trafo = transMat * rotation * scaleMat;
}

string const& Model::getMeshName() const
{
	return meshName;
}

mat4 const& Model::getTrafo() const
{
	return trafo;
}

void Model::setEulerXYZ(vec3 rotation)
{
	orientation = quat(move(rotation));
}

void Model::setPosition(vec3 position)
{
	this->position = move(position);
}

void Model::setScale(vec3 scale)
{
	this->scale = move(scale);
}

void Model::setTrafo(mat4 mat)
{
	trafo = move(mat);
}
