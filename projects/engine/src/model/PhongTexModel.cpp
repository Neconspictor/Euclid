#include <model/PhongTexModel.hpp>

using namespace std;

PhongTexModel::PhongTexModel(const string& meshName, PhongTexMaterial material) : Model(meshName), material(material)
{
}

PhongTexModel::PhongTexModel(const PhongTexModel& other) : Model(other), material(other.material)
{}

PhongTexModel::PhongTexModel(PhongTexModel&& other) : Model(other), material(other.material)
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