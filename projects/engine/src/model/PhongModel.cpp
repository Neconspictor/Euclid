#include <model/PhongModel.hpp>

PhongModel::PhongModel(const std::string& meshName, PhongMaterial material) : Model(meshName),
	material(material)
{}

PhongModel::PhongModel(const PhongModel& other) : Model(other), material(other.material)
{}

PhongModel::PhongModel(PhongModel&& other)
{
}
