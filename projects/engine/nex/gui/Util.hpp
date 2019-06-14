#pragma once
#include "nex/math/Constant.hpp"

namespace nex::gui
{
	void Separator(Real thickness, bool vertical = false);

	void Vector3D(glm::vec3* vec, const char* label, Real speed = 1.0f);

	void EulerRot(glm::vec3* vec, const char* label, Real speed = 1.0f);

	void Quat(glm::quat* quat, const char* label);
}
