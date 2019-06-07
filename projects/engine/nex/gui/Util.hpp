#pragma once

namespace nex::gui
{
	void Separator(float thickness, bool vertical = false);

	void Vector3D(glm::vec3* vec, const char* label, float speed = 1.0f);

	void EulerRot(glm::vec3* vec, const char* label, float speed = 1.0f);

	void Quat(glm::quat* quat, const char* label);
}