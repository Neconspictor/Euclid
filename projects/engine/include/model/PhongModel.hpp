#ifndef ENGINE_MODEL_PHONG_MODEL_HPP
#define ENGINE_MODEL_PHONG_MODEL_HPP
#include <model/Model.hpp>
#include <material/PhongMaterial.hpp>

class PhongModel : public Model
{
public:
	explicit PhongModel(const std::string& meshName, PhongMaterial material);
	PhongModel(const PhongModel& other);
	PhongModel(PhongModel&& other);
	PhongModel& operator=(const PhongModel& other);
	PhongModel& operator=(PhongModel&& other);
	virtual ~PhongModel();

protected:
	PhongMaterial material;
};

#endif