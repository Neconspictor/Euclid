#include <nex/opengl/model/Vob.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/opengl/model/ModelManagerGL.hpp>

using namespace std;
using namespace glm;

namespace nex {

	Vob::Vob(string meshName, ShaderType materialShaderType): m_model(nullptr)
	{
		this->meshName = move(meshName);
		this->materialShaderType = materialShaderType;
		position = {0, 0, 0};
		scale = {1, 1, 1};
		orientation = {1, 0, 0, 0};
	}

	Vob::Vob(ModelGL* model) : m_model(model), materialShaderType(ShaderType::Unknown), position(0, 0, 0), scale(1, 1, 1),
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

	ModelGL* Vob::getModel()
	{
		return m_model;
	}

	ShaderType Vob::getMaterialShaderType() const
	{
		return materialShaderType;
	}

	vec3 Vob::getPosition() const
	{
		return position;
	}

	const mat4& Vob::getTrafo() const
	{
		return trafo;
	}

	void Vob::init(ModelManagerGL* modelManager)
	{
		if (m_model == nullptr)
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

	void Vob::setModel(ModelGL* model)
	{
		m_model = model;
	}
}