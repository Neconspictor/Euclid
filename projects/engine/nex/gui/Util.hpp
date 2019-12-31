#pragma once
#include "nex/math/Constant.hpp"

namespace nex::gui
{
	void Separator(float thickness, bool vertical = false);

	bool Vector3D(glm::vec3* vec, const char* label, float speed = 1.0f);

	void EulerRot(glm::vec3* vec, const char* label, float speed = 1.0f);

	void Quat(glm::quat* quat, const char* label);


	struct ID {
		ID(int id);
		ID(const ID&) = delete;
		ID(ID&&) = delete;
		ID& operator=(const ID&) = delete;
		ID& operator=(ID&&) = delete;
		~ID();
	};
}
