#include <nex/model/Vob.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

Vob::Vob(string meshName, Shaders materialShaderType)
{
	this->meshName = move(meshName);
	this->materialShaderType = materialShaderType;
	position = { 0,0,0 };
	scale = { 1,1,1 };
	orientation = { 1,0,0,0 };
}

Vob::Vob(const Vob& other) : meshName(other.meshName), 
materialShaderType(other.materialShaderType),
orientation(other.orientation), 
position(other.position),
scale(other.scale), 
trafo(other.trafo)
{}

Vob::Vob(Vob&& other) : meshName(other.meshName), 
materialShaderType(other.materialShaderType),
orientation(other.orientation), 
position(other.position),
scale(other.scale), 
trafo(other.trafo)
{}

Vob& Vob::operator=(const Vob& other)
{
	meshName = other.meshName;
	materialShaderType = other.materialShaderType;
	orientation = other.orientation;
	position = other.position;
	scale = other.scale;
	trafo = other.trafo;
	return *this;
}

Vob& Vob::operator=(Vob&& other)
{
	meshName = move(other.meshName);
	materialShaderType = other.materialShaderType;
	orientation = move(other.orientation);
	position = move(other.position);
	scale = move(other.scale);
	trafo = move(other.trafo);
	return *this;
}

Vob::~Vob()
{
}

void Vob::calcTrafo()
{
	trafo = mat4();
	mat4 rotation = mat4_cast(orientation);
	mat4 scaleMat = glm::scale(trafo, scale);
	mat4 transMat = translate(trafo, position);
	trafo = transMat * rotation * scaleMat;
}

string const& Vob::getMeshName() const
{
	return meshName;
}

Model* Vob::getModel()
{
	return m_model;
}

Shaders Vob::getMaterialShaderType() const
{
	return materialShaderType;
}

vec3 Vob::getPosition() const
{
	return position;
}

mat4& Vob::getTrafo()
{
	return trafo;
}

void Vob::init(ModelManager* modelManager)
{
	m_model = modelManager->getModel(meshName, materialShaderType);
}

void Vob::setEulerXYZ(vec3 rotation)
{
	orientation = quat(move(rotation));
}

void Vob::setPosition(vec3 position)
{
	this->position = move(position);
}

void Vob::setScale(vec3 scale)
{
	this->scale = move(scale);
}

void Vob::setTrafo(mat4 mat)
{
	trafo = move(mat);
}