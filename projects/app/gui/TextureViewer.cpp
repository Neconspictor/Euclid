#include <gui/TextureViewer.hpp>
#include <imgui/imgui.h>
#include "nex/gui/Util.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <nfd/nfd.h>
#include <nex/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <boxer/boxer.h>
#include <nex/Scene.hpp>
#include <nex/texture/Image.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/TextureManager.hpp>
#include <imgui/imgui.h>

namespace nex::gui
{
	TextureViewer::TextureViewer(std::string title, nex::gui::MainMenuBar* menuBar, nex::gui::Menu* menu, nex::Window* widow) :
		MenuWindow(std::move(title), menuBar, menu),
		mDynamicLoad({}, ImVec2(256, 256)),
		mWindow(widow)
	{
	}

	TextureViewer::~TextureViewer() = default;

	void nex::gui::TextureViewer::drawSelf()
	{		
		static Future<Resource*> textureFuture;
		Texture* loadedTexture = nullptr;

		if (textureFuture.is_ready())
			loadedTexture = (Texture*)textureFuture.get();

		if (loadedTexture) {

			auto& desc = mDynamicLoad.getTexture();
			desc.texture = loadedTexture;
			desc.flipY = nex::ImageFactory::isYFlipped();
			desc.sampler = Sampler::getDefaultImage();

			mDynamicLoad.updateTexture(true);
			mDynamicLoad.drawGUI();
		}


		if (ImGui::Button("Load Image")) {

			if (textureFuture.is_ready() || !textureFuture.valid()) {

				textureFuture = ResourceLoader::get()->enqueue([=](RenderEngine::CommandQueue* commandQueue)->nex::Resource * {

					FileDialog fileDialog(mWindow);
					auto result = fileDialog.selectFile("jpg,png,psd,bpm,tga,hdr");

					TextureDesc data;
					data.colorspace = ColorSpace::SRGBA;
					data.internalFormat = InternFormat::SRGBA8;
					data.generateMipMaps = true;

					switch (result.state) {
					case FileDialog::State::Okay:
						std::cout << "Selected file: " << result.path << std::endl;
						return TextureManager::get()->getImage(result.path, data, true);
						break;
					case FileDialog::State::Cancled:
						std::cout << "Canceled" << std::endl;
						break;
					case FileDialog::State::Error:
						std::cout << "Error: " << result.error << std::endl;
						break;
					}

					return nullptr;
					});
			}
		}     
	}
}