#include <nex/gui/vob/VobLoader.hpp>
#include <imgui/imgui.h>
#include "nex/gui/ImGUI_Extension.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <nfd/nfd.h>
#include <nex/platform/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <boxer/boxer.h>
#include <nex/scene/Scene.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/material/PbrMaterialLoader.hpp>
#include <nex/pbr/Pbr.hpp>
#include <nex/pbr/PbrDeferred.hpp>
#include <nex/pbr/PbrForward.hpp>
#include <nex/pbr/PbrPass.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/anim/AnimationManager.hpp>
#include <nex/util/Memory.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/scene/VobBluePrint.hpp>

namespace nex::gui
{
	VobLoader::VobLoader(std::string title, 
		nex::gui::MainMenuBar* menuBar, 
		nex::gui::Menu* menu, 
		nex::Scene* scene, 
		VobBluePrints* bluePrints,
		nex::PbrTechnique* pbrTechnique,
		nex::Window* widow,
		Camera* camera) :
		MenuWindow(std::move(title), menuBar, menu),
		mScene(scene),
		mBluePrints(bluePrints),
		mWindow(widow),
		mPbrTechnique(pbrTechnique),
		mCamera(camera)
	{
	}

	VobLoader::~VobLoader() = default;

	void VobLoader::setScene(nex::Scene * scene)
	{
		mScene = scene;
	}

	void nex::gui::VobLoader::drawSelf()
	{
		Vob* loadedVob = nullptr;

		if (mFuture.is_ready()) {
			loadedVob = mFuture.get();
			mFuture = Future<Vob*>();
		}

		if (mFuture.valid()) {
			ImGui::Text("Loading...");
			ImGui::SameLine();
			//bool nex::gui::Spinner(const char* label, float radius, int thickness, const ImU32 & color)
			nex::gui::Spinner("##loading_spinner", 8.0f, 3, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
		}

		if (loadedVob) {
			loadedVob = nullptr;
			
			//TODO
			//boxer::show("Loaded vob successfully!", "Vob loader", boxer::Style::Info, boxer::Buttons::OK, mWindow->getNativeWindow());
		}
			
		ImGui::Checkbox("Use rescale", &mUseRescale);
		if (mUseRescale)
			ImGui::InputFloat("Default scale", (float*)&mDefaultScale);

		if (ImGui::Button("Load visual object")) {

			if (mFuture.is_ready() || !mFuture.valid()) {
				selectAndLoadVob();
			}
		}
	}

	void VobLoader::selectAndLoadVob()
	{
		FileDialog fileDialog(mWindow);
		auto result = fileDialog.selectFile("obj,gltf,glb,fbx");

		if (result.state == FileDialog::State::Okay) {
			//std::cout << "Selected file: " << result.path << std::endl;


			mFuture = ResourceLoader::get()->enqueue<nex::Vob*>([=]()->Vob* {
				nex::flexible_ptr<Vob> vob = nullptr;

				try {
					auto* deferred = mPbrTechnique->getDeferred();
					auto* forward = mPbrTechnique->getForward();

					PbrMaterialLoader materialLoader(deferred->getGeometryShaderProvider(),
						deferred->getGeometryBonesShaderProvider(),
						forward->getShaderProvider(),
						forward->getBoneShaderProvider(),
						TextureManager::get());

					vob = loadVob(result.path.u8string(),
						RenderEngine::getCommandQueue(),
						materialLoader);
				}
				catch (std::exception & e) {
					void* nativeWindow = mWindow->getNativeWindow();

					nex::ExceptionHandling::logExceptionWithStackTrace(Logger("VobLoader"), e);

					auto msg = std::string("Couldn't load mesh. See console output for more details ");


					boxer::show(msg.c_str(), "", boxer::Style::Error, boxer::Buttons::OK, nativeWindow);
					return nullptr;
				}

				// TODO: Until the task is excuted, the vob's memory isn't managed.
				// It is unlikely but this could result into a memory leak if the task is not executed (e.g. due a thrown exception)

				RenderEngine::getCommandQueue()->push([=, ptr = vob.get()]() {

					//Note: flex is const
					nex::flexible_ptr<Vob> vob = nex::make_owning(ptr);

					vob->finalizeMeshes();
					auto lock = mScene->acquireLock();

					//auto rescaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(mUseRescale ? mDefaultScale : 1.0f));
					//auto rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
					// Now apply rescale
					//vob->setTrafoMeshToLocal(rescaleMatrix);

					vob->setPositionLocalToParent(mCamera->getPosition() + 1.0f * mCamera->getLook());
					vob->setScaleLocalToParent(vob->getScaleLocalToParent() * glm::vec3(mUseRescale ? mDefaultScale : 1.0f));
					vob->updateWorldTrafoHierarchy(true);

					mScene->addVobUnsafe(std::unique_ptr<Vob>(vob.release()));

					});

				return vob.release();
			});
		}
	}

	std::unique_ptr<nex::Vob> nex::gui::VobLoader::loadVob(const std::filesystem::path& p,
		nex::RenderEngine::CommandQueue* commandQueue,
		const AbstractMaterialLoader& materialLoader,
		const nex::FileSystem* fileSystem)
	{
		if (!fileSystem) fileSystem = &MeshManager::get()->getFileSystem();

		auto path = fileSystem->resolvePath(p);
		auto id = SID(path.generic_string());
		auto* bluePrintPtr = mBluePrints->getCachedPtr(id);

		if (!bluePrintPtr) {
			auto vob = MeshManager::get()->loadVobHierarchy(path, materialLoader, 1.0f);

			commandQueue->push([vobPtr = vob.get()]() {
				vobPtr->finalizeMeshes();
			});

			auto bluePrint = std::make_unique<VobBluePrint>(std::move(vob));
			bluePrintPtr = bluePrint.get();

			mBluePrints->insert(id, std::move(bluePrint));
		}

		return bluePrintPtr->createBluePrint();
	}
}