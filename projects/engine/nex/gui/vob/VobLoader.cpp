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
				std::unique_ptr<Vob> vob = nullptr;

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
				catch (std::exception& e) {
					void* nativeWindow = mWindow->getNativeWindow();
					
					nex::ExceptionHandling::logExceptionWithStackTrace(Logger("VobLoader"), e);
					
					auto msg = std::string("Couldn't load mesh. See console output for more details ");


					boxer::show(msg.c_str(), "", boxer::Style::Error, boxer::Buttons::OK, nativeWindow);
					return nullptr;
				}
				
				RenderEngine::getCommandQueue()->push([=, vobPtr = vob.get()]() {

					std::unique_ptr<Vob> vob(vobPtr);

					vob->finalizeMeshes();
					auto lock = mScene->acquireLock();

					auto rescaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(mUseRescale ? mDefaultScale : 1.0f));
					//auto rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
					// Now apply rescale
					vob->setTrafoMeshToLocal(rescaleMatrix);

					vob->setPositionLocalToParent(mCamera->getPosition() + 1.0f * mCamera->getLook());
					vob->updateWorldTrafoHierarchy(true);

					mScene->addVobUnsafe(std::move(vob));
				});

				vob.release();
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
				std::unique_ptr<Vob> vob = nullptr;

				try {
					auto* deferred = mPbrTechnique->getDeferred();
					auto* forward = mPbrTechnique->getForward();

					PbrMaterialLoader materialLoader(deferred->getGeometryShaderProvider(),
						deferred->getGeometryBonesShaderProvider(),
						forward->getShaderProvider(),
						forward->getBoneShaderProvider(),
						TextureManager::get());

					nex::SkinnedMeshLoader meshLoader;
					auto* fileSystem = nex::AnimationManager::get()->getRiggedMeshFileSystem();

					vob = loadVob(result.path.u8string(),
						RenderEngine::getCommandQueue(), 
						materialLoader,
						fileSystem);
				}
				catch (std::exception & e) {
					void* nativeWindow = mWindow->getNativeWindow();

					nex::ExceptionHandling::logExceptionWithStackTrace(Logger("VobLoader"), e);

					auto msg = std::string("Couldn't load mesh. See console output for more details ");


					boxer::show(msg.c_str(), "", boxer::Style::Error, boxer::Buttons::OK, nativeWindow);
					return nullptr;
				}

			
				RenderEngine::getCommandQueue()->push([=, vobPtr = vob.get()]() {

					std::unique_ptr<Vob> vob (vobPtr);

					vob->finalizeMeshes();
					auto lock = mScene->acquireLock();

					auto rescaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(mUseRescale ? mDefaultScale : 1.0f));
					//auto rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
					// Now apply rescale
					vob->setTrafoMeshToLocal(rescaleMatrix);

					vob->setPositionLocalToParent(mCamera->getPosition() + 1.0f * mCamera->getLook());
					vob->updateWorldTrafoHierarchy(true);

					mScene->addVobUnsafe(std::move(vob));
				});

				vob.release();

				return nullptr;
			}

			return nullptr;
			});
	}

	std::unique_ptr<nex::Vob> nex::gui::VobLoader::loadVob(const std::filesystem::path& p,
		nex::RenderEngine::CommandQueue* commandQueue,
		const AbstractMaterialLoader& materialLoader,
		const nex::FileSystem* fileSystem)
	{
		if (!fileSystem) fileSystem = &MeshManager::get()->getFileSystem();

		auto path = fileSystem->resolvePath(p);
		auto id = SID(path.generic_string());
		auto* bluePrint = mBluePrints->getCachedPtr(id);

		if (!bluePrint) {
			auto vob = MeshManager::get()->loadVobHierarchy(path, materialLoader, 1.0f);
			bluePrint = vob.get();

			commandQueue->push([bluePrint = bluePrint]() {
				bluePrint->finalizeMeshes();
				});

			mBluePrints->insert(id, std::move(vob));
		}

		return bluePrint->createBluePrintCopy();
	}
}