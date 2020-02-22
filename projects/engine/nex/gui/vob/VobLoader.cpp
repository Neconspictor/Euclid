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
#include <nex/pbr/PbrPass.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/anim/AnimationManager.hpp>

namespace nex::gui
{
	VobLoader::VobLoader(std::string title, 
		nex::gui::MainMenuBar* menuBar, 
		nex::gui::Menu* menu, 
		nex::Scene* scene, 
		std::vector<std::unique_ptr<MeshGroup>>* meshes,
		nex::PbrTechnique* pbrTechnique,
		nex::Window* widow,
		Camera* camera) :
		MenuWindow(std::move(title), menuBar, menu),
		mScene(scene),
		mMeshes(meshes),
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

	void VobLoader::setMeshes(std::vector<std::unique_ptr<nex::MeshGroup>>* meshes)
	{
		mMeshes = meshes;
	}

	void nex::gui::VobLoader::drawSelf()
	{
		static Future<Resource*> meshFuture;
		MeshGroup* loadedMesh = nullptr;

		if (meshFuture.is_ready())
			loadedMesh = (MeshGroup*)meshFuture.get();

		ImGui::Checkbox("Use Rescale", &mUseRescale);
		if (mUseRescale)
			ImGui::InputFloat("Default Scale", (float*)&mDefaultScale);

		if (ImGui::Button("Load Mesh")) {

			if (meshFuture.is_ready() || !meshFuture.valid()) {
				meshFuture = loadStaticMesh();
			}
		}

		if (ImGui::Button("Load Rigged Mesh")) {

			if (meshFuture.is_ready() || !meshFuture.valid()) {
				meshFuture = loadRiggedMesh();
			}
		}
	}

	nex::Future<Resource*> VobLoader::loadStaticMesh()
	{
		return ResourceLoader::get()->enqueue([=]()->nex::Resource* {
			FileDialog fileDialog(mWindow);
			auto result = fileDialog.selectFile("obj");

			if (result.state == FileDialog::State::Okay) {
				//std::cout << "Selected file: " << result.path << std::endl;
				MeshGroup* groupPtr = nullptr;

				try {

					auto* deferred = mPbrTechnique->getDeferred();

					PbrMaterialLoader solidMaterialLoader(deferred->getGeometryShaderProvider(), TextureManager::get());
					auto group = MeshManager::get()->loadModel(result.path.u8string(), solidMaterialLoader,
						mUseRescale ? mDefaultScale : 1.0f,
						true);
					groupPtr = group.get();
					mMeshes->emplace_back(std::move(group));

				}
				catch (std::exception& e) {
					void* nativeWindow = mWindow->getNativeWindow();
					
					nex::ExceptionHandling::logExceptionWithStackTrace(Logger("VobLoader"), e);
					
					auto msg = std::string("Couldn't load mesh. See console output for more details ");


					boxer::show(msg.c_str(), "", boxer::Style::Error, boxer::Buttons::OK, nativeWindow);
					return nullptr;
				}
				
				RenderEngine::getCommandQueue()->push([=]() {
					groupPtr->finalize();
					auto lock = mScene->acquireLock();
					auto* vob = mScene->createVobUnsafe(groupPtr);

					vob->setPositionLocalToParent(mCamera->getPosition() + 1.0f * mCamera->getLook());
					vob->updateWorldTrafoHierarchy(true);
					});

				return groupPtr;
			}

			return nullptr;
		});
	}

	nex::Future<Resource*> VobLoader::loadRiggedMesh()
	{
		return ResourceLoader::get()->enqueue([=]()->nex::Resource* {
			FileDialog fileDialog(mWindow);
			auto result = fileDialog.selectFile("md5mesh");

			if (result.state == FileDialog::State::Okay) {
				//std::cout << "Selected file: " << result.path << std::endl;
				MeshGroup* groupPtr = nullptr;

				try {
					auto* deferred = mPbrTechnique->getDeferred();
					PbrMaterialLoader solidBoneAlphaStencilMaterialLoader(deferred->getGeometryBonesShaderProvider(), TextureManager::get(),
						PbrMaterialLoader::LoadMode::SOLID_ALPHA_STENCIL);

					nex::SkinnedMeshLoader meshLoader;
					auto* fileSystem = nex::AnimationManager::get()->getRiggedMeshFileSystem();

					auto group = MeshManager::get()->loadModel(result.path.u8string(), solidBoneAlphaStencilMaterialLoader,
						1.0f, // Note: We don't have to rescale a rigged mesh at this point, since bone transformations than wouldn't work anymore.
						true,
						&meshLoader,
						fileSystem);
					groupPtr = group.get();
					mMeshes->emplace_back(std::move(group));
				}
				catch (std::exception & e) {
					void* nativeWindow = mWindow->getNativeWindow();

					nex::ExceptionHandling::logExceptionWithStackTrace(Logger("VobLoader"), e);

					auto msg = std::string("Couldn't load mesh. See console output for more details ");


					boxer::show(msg.c_str(), "", boxer::Style::Error, boxer::Buttons::OK, nativeWindow);
					return nullptr;
				}

				RenderEngine::getCommandQueue()->push([=]() {
					groupPtr->finalize();
					auto lock = mScene->acquireLock();
					auto vob = std::make_unique<RiggedVob>(nullptr);
					vob->setMeshGroup(groupPtr);


					auto rescaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(mUseRescale ? mDefaultScale : 1.0f));
					//auto rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
					// Now apply rescale
					vob->setTrafoMeshToLocal(rescaleMatrix);

					vob->setPositionLocalToParent(mCamera->getPosition() + 1.0f * mCamera->getLook());
					vob->updateWorldTrafoHierarchy(true);
					mScene->addVobUnsafe(std::move(vob));

					});

				return groupPtr;
			}

			return nullptr;
			});
	}
}