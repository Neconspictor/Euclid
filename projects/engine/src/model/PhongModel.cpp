#include <model/PhongModel.hpp>

using namespace std;

PhongModel::PhongModel(const std::string& meshName, PhongMaterial material) : Model(meshName),
	material(material)
{}

PhongModel::PhongModel(const PhongModel& other) : Model(other), material(other.material)
{}

PhongModel::PhongModel(PhongModel&& other) : Model(other), material(other.material)
{}

PhongModel& PhongModel::operator=(const PhongModel& other)
{
	material = other.material;
	return *this;
}

PhongModel& PhongModel::operator=(PhongModel&& other)
{
	material = move(other.material);
	return *this;
}

PhongModel::~PhongModel()
{
}

const PhongMaterial& PhongModel::getMaterial()
{
	return material;
}