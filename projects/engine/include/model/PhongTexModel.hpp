#ifndef ENGINE_MODEL_PHONG_TEX__MODEL_HPP
#define ENGINE_MODEL_PHONG_TEX_MODEL_HPP
#include <model/Model.hpp>
#include <material/PhongTexMaterial.hpp>

class PhongTexModel : public Model
{
public:
	explicit PhongTexModel(const std::string& meshName, PhongTexMaterial material);
	PhongTexModel(const PhongTexModel& other);
	PhongTexModel(PhongTexModel&& other);
	PhongTexModel& operator=(const PhongTexModel& other);
	PhongTexModel& operator=(PhongTexModel&& other);
	virtual ~PhongTexModel();

	const PhongTexMaterial& getMaterial();

protected:
	PhongTexMaterial material;
};

#endif