#include <gui/VobLoader.hpp>
#include <imgui/imgui.h>
#include "nex/gui/Util.hpp"
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

namespace nex::gui
{
	VobLoader::VobLoader(std::string title, 
		nex::gui::MainMenuBar* menuBar, 
		nex::gui::Menu* menu, 
		nex::Scene* scene, 
		nex::PbrTechnique* pbrTechnique,
		nex::Window* widow) :
		MenuWindow(std::move(title), menuBar, menu),
		mScene(scene),
		mWindow(widow),
		mPbrTechnique(pbrTechnique)
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

		if (ImGui::Button("Load Mesh")) {

			if (meshFuture.is_ready() || !meshFuture.valid()) {
				meshFuture = ResourceLoader::get()->enqueue([=](RenderEngine::CommandQueue* commandQueue)->nex::Resource * {

					FileDialog fileDialog(mWindow);
					auto result = fileDialog.selectFile("obj");

					if (result.state == FileDialog::State::Okay) {
						std::cout << "Selected file: " << result.path << std::endl;
						MeshGroup* meshContainer = nullptr;

						try {

							auto* deferred = mPbrTechnique->getDeferred();

							PbrMaterialLoader solidMaterialLoader(deferred->getGeometryShader(), TextureManager::get());
							meshContainer = MeshManager::get()->loadModel(result.path.u8string(), solidMaterialLoader);
						}
						catch (...) {
							void* nativeWindow = mWindow->getNativeWindow();
							boxer::show("Couldn't load mesh!", "", boxer::Style::Error, boxer::Buttons::OK, nativeWindow);
							return nullptr;
						}

						commandQueue->push([=]() {
							meshContainer->finalize();
							auto lock = mScene->acquireLock();
							auto* nodes = meshContainer->createNodeHierarchyUnsafe();
							auto* vob = mScene->createVobUnsafe(nodes);
							vob->setPosition(glm::vec3(-9.0f, 2.0f, 4.0f));
							vob->getMeshRootNode()->updateWorldTrafoHierarchy(true);
							});

						return meshContainer;
					}

					return nullptr;
					});
			}
		}
	}
}