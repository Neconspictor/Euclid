#pragma once
#include <model/Vob.hpp>
#include <material/PhongMaterial.hpp>

class PhongModel : public Vob
{
public:
	explicit PhongModel(const std::string& meshName, PhongMaterial material);
	PhongModel(const PhongModel& other);
	PhongModel(PhongModel&& other);
	PhongModel& operator=(const PhongModel& other);
	PhongModel& operator=(PhongModel&& other);
	virtual ~PhongModel();

	const PhongMaterial& getMaterial();

protected:
	PhongMaterial material;
};