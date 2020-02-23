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
#include <nex/util/Memory.hpp>
#include <nex/mesh/MeshGroup.hpp>

namespace nex::gui
{
	VobLoader::VobLoader(std::string title, 
		nex::gui::MainMenuBar* menuBar, 
		nex::gui::Menu* menu, 
		nex::Scene* scene, 
		MeshCache* meshCache,
		nex::PbrTechnique* pbrTechnique,
		nex::Window* widow,
		Camera* camera) :
		MenuWindow(std::move(title), menuBar, menu),
		mScene(scene),
		mMeshCache(meshCache),
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

	void VobLoader::setMeshCache(MeshCache* meshCache)
	{
		mMeshCache = meshCache;
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

					PbrMaterialLoader solidMaterialLoader(deferred->getGeometryShaderProvider(), deferred->getGeometryBonesShaderProvider(), TextureManager::get());

					groupPtr = loadMeshGroup(result.path.u8string(),
						RenderEngine::getCommandQueue(),
						solidMaterialLoader);

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

					
					auto rescaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(mUseRescale ? mDefaultScale : 1.0f));
					//auto rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
					// Now apply rescale
					vob->setTrafoMeshToLocal(rescaleMatrix);
					
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
					PbrMaterialLoader solidBoneAlphaStencilMaterialLoader(deferred->getGeometryShaderProvider(), deferred->getGeometryBonesShaderProvider(), TextureManager::get(),
						PbrMaterialLoader::LoadMode::SOLID_ALPHA_STENCIL);

					nex::SkinnedMeshLoader meshLoader;
					auto* fileSystem = nex::AnimationManager::get()->getRiggedMeshFileSystem();

					groupPtr = loadMeshGroup(result.path.u8string(), 
						RenderEngine::getCommandQueue(), 
						solidBoneAlphaStencilMaterialLoader, 
						&meshLoader, 
						fileSystem);
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
					auto vob = std::make_unique<RiggedVob>();
					vob->setMeshGroup(nex::make_not_owning(groupPtr));


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
	nex::MeshGroup* VobLoader::loadMeshGroup(const std::filesystem::path& p, 
		nex::RenderEngine::CommandQueue* commandQueue, 
		const AbstractMaterialLoader& materialLoader, 
		nex::AbstractMeshLoader* loader, 
		const nex::FileSystem* fileSystem)
	{
		if (!fileSystem) fileSystem = &MeshManager::get()->getFileSystem();

		auto path = fileSystem->resolvePath(p);
		auto id = SID(path.generic_string());
		MeshGroup* groupPtr = mMeshCache->getCachedPtr(id);

		if (!groupPtr) {
			auto group = MeshManager::get()->loadModel(path, materialLoader, 1.0f, false, loader, fileSystem);
			groupPtr = group.get();

			commandQueue->push([groupPtr = group.get()]() {
				groupPtr->finalize();
			});

			mMeshCache->insert(id, std::move(group));
		}

		return groupPtr;
	}
}