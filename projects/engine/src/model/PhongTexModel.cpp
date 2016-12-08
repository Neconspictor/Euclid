#include <model/PhongTexModel.hpp>

using namespace std;

PhongTexModel::PhongTexModel(const string& meshName, PhongTexMaterial material) : Vob(meshName), material(material)
{
}

PhongTexModel::PhongTexModel(const PhongTexModel& other) : Vob(other), material(other.material)
{}

PhongTexModel::PhongTexModel(PhongTexModel&& other) : Vob(other), material(other.material)
{}

PhongTexModel& PhongTexModel::operator=(const PhongTexModel& other)
{
	material = other.material;
	return *this;
}

PhongTexModel& PhongTexModel::operator=(PhongTexModel&& other)
{
	material = move(other.material);
	return *this;
}

PhongTexModel::~PhongTexModel()
{
}

const PhongTexMaterial& PhongTexModel::getMaterial()
{
	return material;
}