#include <nex/mesh/Vob.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/material/Material.hpp>

using namespace std;
using namespace glm;

namespace nex {

	Vob::Vob(string meshName, MaterialType materialType): m_model(nullptr),
		mMaterialType(materialType),
		meshName(move(meshName))
	{
		position = {0, 0, 0};
		scale = {1, 1, 1};
		orientation = {1, 0, 0, 0};
	}

	Vob::Vob(StaticMeshContainer* model) : m_model(model), mMaterialType(MaterialType::None), position(0, 0, 0), scale(1, 1, 1),
	                           orientation(1, 0, 0, 0)
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

	StaticMeshContainer* Vob::getModel()
	{
		return m_model;
	}

	MaterialType Vob::getMaterialType() const
	{
		return mMaterialType;
	}

	vec3 Vob::getPosition() const
	{
		return position;
	}

	const mat4& Vob::getTrafo() const
	{
		return trafo;
	}

	void Vob::init()
	{

		static auto* manager = StaticMeshManager::get();
		if (m_model == nullptr)
			m_model = manager->getModel(meshName);
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

	void Vob::setModel(StaticMeshContainer* model)
	{
		m_model = model;
	}
}