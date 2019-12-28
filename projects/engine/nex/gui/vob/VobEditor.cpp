#include <nex/gui/vob/VobEditor.hpp>
#include <imgui/imgui.h>
#include "nex/gui/Util.hpp"
#include "nex/gui/Picker.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "nex/pbr/PbrProbe.hpp"
#include "nex/texture/TextureManager.hpp"
#include <nfd/nfd.h>
#include <nex/platform/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <boxer/boxer.h>
#include <nex/gui/vob/VobView.hpp>

namespace nex::gui
{
	VobEditor::VobEditor(nex::Window* window, Picker* picker) :
		mWindow(window),
		mScene(nullptr),
		mLastPickedVob(nullptr),
		mPicker(picker),
		mVobView(nullptr)
		//mTransparentView({}, ImVec2(256, 256))
	{	
	}

	VobEditor::~VobEditor() = default;

	void VobEditor::setScene(nex::Scene * scene)
	{
		mScene = scene;
	}

	void VobEditor::setVobView(VobView* view)
	{
		mVobView = view;
	}


	// Calculates rotation matrix given euler angles.
	glm::mat3 eulerAnglesToRotationMatrix(const glm::vec3& theta)
	{
		// Calculate rotation about x axis
		glm::mat3 R_x = {
			1, 0, 0,
			0, cos(theta[0]), -sin(theta[0]),
			0, sin(theta[0]), cos(theta[0])
		};

		// Calculate rotation about y axis
		glm::mat3 R_y = {
			cos(theta[1]), 0, sin(theta[1]),
			0, 1, 0,
			-sin(theta[1]), 0, cos(theta[1])
		};

		// Calculate rotation about z axis
		glm::mat3 R_z = {
			cos(theta[2]), -sin(theta[2]), 0,
			sin(theta[2]), cos(theta[2]), 0,
			0, 0, 1 };


		// Combined rotation matrix
		return transpose(R_z) * transpose(R_y) * transpose(R_x);
	}

	// Calculates rotation matrix to euler angles
// The result is the same as MATLAB except the order
// of the euler angles ( x and z are swapped ).
	glm::vec3 rotationMatrixToEulerAngles(const glm::mat4 &R)
	{
		float sy = sqrt(R[0][0] * R[0][0] + R[0][1] * R[0][1]);

		bool singular = sy < 1e-6; // If

		float x, y, z;
		if (!singular)
		{
			x = atan2(R[1][2], R[2][2]);
			y = atan2(-R[0][2], sy);
			z = atan2(R[0][1], R[0][0]);
		}
		else
		{
			x = atan2(-R[2][1], R[1][1]);
			y = atan2(-R[0][2], sy);
			z = 0;
		}
		return { x, y, z };
	}

	void nex::gui::VobEditor::drawSelf()
	{
		nex::gui::Separator(2.0f);

		Vob* vob = mPicker->getPicked();
		bool doOneTimeChanges = vob != mLastPickedVob;
		mLastPickedVob = vob;

		if (!vob) {
			ImGui::Text("No scene node selected.");
			return;
		}

		if (!mVobView) {
			ImGui::Text("No view found.");
			return;
		}

		mVobView->draw(vob, mScene, mPicker, doOneTimeChanges);
	}
}