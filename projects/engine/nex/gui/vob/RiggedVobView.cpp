#include <nex/gui/vob/RiggedVobView.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/anim/Rig.hpp>
#include <nex/anim/BoneAnimation.hpp>
#include <nex/anim/AnimationManager.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <boxer/boxer.h>
#include <nex/platform/Window.hpp>

nex::gui::RiggedVobView::RiggedVobView(nex::Window* window) : VobView(window)
{
}

bool nex::gui::RiggedVobView::draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges)
{
	if (!VobView::draw(vob, scene, picker, camera, doOneTimeChanges)) {
		return false;
	}

	auto* riggedVob = dynamic_cast<RiggedVob*>(vob);

	if (!riggedVob) {
		return true;
	}

	auto* rig = riggedVob->getRig();

	ImGui::Separator();

	ImGui::Text("Rig ID: %s", rig->getID().c_str());



	const auto* animation = riggedVob->getActiveAnimation();


	auto* animationManager = nex::AnimationManager::get();
	const auto& aniSet = animationManager->getBoneAnimationsByRig(rig);
	auto animations =  std::vector<const nex::BoneAnimation*>(begin(aniSet), end(aniSet));
	std::sort(begin(animations), end(animations), [](const auto* a, const auto* b) {
		return std::lexicographical_compare(begin(a->getName()), end(a->getName()), 
												begin(b->getName()), end(b->getName()));
	});

	std::vector<const char*> animationNames(animations.size());

	for (int i = 0; i < animations.size(); ++i) {
		const auto* ani = animations[i];
		animationNames[i] = ani->getName().c_str();
	}

	// Note: ImGui handles out of range indices properly with an empty field.
	// So it is safe to use an invalid index.
	int activeAnimatinIndex = -1;

	if (animation) {
		for (int i = 0; i < animations.size(); ++i) {
			if (animations[i] == animation) {
				activeAnimatinIndex = i;
				break;
			}
		}
	}

	if (ImGui::Combo("Active animation", &activeAnimatinIndex, animationNames.data(), animationNames.size())) {
		if (activeAnimatinIndex >= 0) {
			auto* newAni = animations[activeAnimatinIndex];
			riggedVob->setActiveAnimation(newAni);
		}
	}

	ImGui::SameLine();
	drawLoadAni();


	const bool isPaused = riggedVob->isAnimationPaused();

	const char* playText[2] = {"Pause", "Resume"};
	if (ImGui::Button(playText[(int)isPaused])) {
		riggedVob->pauseAnimation(!isPaused);
	}


	return true;
}

void nex::gui::RiggedVobView::drawLoadAni()
{
	if (ImGui::Button("Load Animation")) {
		mResourceFuture = loadAnimation();
	}
}

nex::Future<nex::Resource*> nex::gui::RiggedVobView::loadAnimation()
{
	return nex::ResourceLoader::get()->enqueue<nex::Resource*>([=]()->nex::Resource* {
		nex::gui::FileDialog fileDialog(mWindow);
		auto result = fileDialog.selectFile("gltf,glb,md5anim");

		if (result.state == FileDialog::State::Okay) {
			//BoneAnimation* ani = nullptr;

			try {

				AnimationManager::get()->loadBoneAnimations(result.path.u8string());
			}

			catch (const std::exception& e) {

				nex::ExceptionHandling::logExceptionWithStackTrace(Logger("RiggedVobView::loadAnimation"), e);

				void* nativeWindow = mWindow->getNativeWindow();
				boxer::show("Couldn't load bone animation!", "", boxer::Style::Error, boxer::Buttons::OK, nativeWindow);
			}

			catch (...) {
				void* nativeWindow = mWindow->getNativeWindow();
				boxer::show("Couldn't load bone animation!", "", boxer::Style::Error, boxer::Buttons::OK, nativeWindow);
			}
		}

		return nullptr;
		});
}



/*
Editable text test stuff:
static std::string editableText = "editable text";

	static bool active = false;
	static bool activate = false;
	static bool oneFrameAfter = false;


	auto flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;





	//if (!active) flags |= ImGuiInputTextFlags_ReadOnly;
	auto& io = ImGui::GetIO();
	ImGuiContext& g = *GImGui;

	if (oneFrameAfter) {
		bool test = true;
		oneFrameAfter = false;
	}


	if (activate) {
		io.MouseClicked[0] = true;
		io.MouseDown[0] = true;
		io.MouseDownWasDoubleClick[0] = false;
		io.MouseDoubleClicked[0] = false;
		auto id = ImGui::GetCurrentWindow()->GetID("##Input field");
		//ImGui::ActivateItem(id);
		//ImGui::SetHoveredID(id);
		//ImGui::SetActiveID(id, ImGui::GetCurrentWindow());

		g.ActiveIdAllowOverlap = true;
		g.HoveredIdAllowOverlap = true;
		g.NavInputId = id;
		oneFrameAfter = true;

		//activate = false;
	}

	if (active) {
		if (ImGui::InputText("##Input field", &editableText, flags)) {
			LOG(Logger("RiggedVobView"), Info) << "input field returned true";
			active = false;
		}

		auto& state = g.InputTextState;

		if (state.HasSelection()) {
			bool test = true;
		}

		ImGuiContext& g = *GImGui;
		//g.InputTextState.SelectAll();
		//g.InputTextState.SelectedAllMouseLock = true;

		if (activate) {
			ImGuiContext& g = *GImGui;
			//g.InputTextState.SelectAll();
			//g.InputTextState.SelectedAllMouseLock = true;
			auto id = ImGui::GetCurrentWindow()->GetID("##Input field");

			ImGui::ActivateItem(id);
			activate = false;
		}

		auto id = ImGui::GetCurrentWindow()->GetID("##Input field");
		ImGui::ActivateItem(id);

		if (ImGui::IsMouseClicked(1)) {
			//activate = true;
			ImGui::MarkItemEdited(id);
			active = false;
		}
	}
	else {

		ImGui::Text(editableText.c_str());
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
			activate = true;
			active = true;
		}
	}


	rig->getBones();


	static glm::vec3 value;
	ImGui::DragFloat3("test", (float*)&value);

*/