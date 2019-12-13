#include <gui/TextureViewer.hpp>
#include <imgui/imgui.h>
#include "nex/gui/Util.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <nfd/nfd.h>
#include <nex/platform/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <boxer/boxer.h>
#include <nex/scene/Scene.hpp>
#include <nex/texture/Image.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/TextureManager.hpp>
#include <imgui/imgui.h>
#include <boxer/boxer.h>

namespace nex::gui
{
	TextureViewer::TextureViewer(std::string title, nex::gui::MainMenuBar* menuBar, nex::gui::Menu* menu, nex::Window* widow) :
		MenuWindow(std::move(title), menuBar, menu),
		mDynamicLoad({}, ImVec2(256, 256)),
		mWindow(widow)
	{
		mDynamicLoad.showFilteringConfig(false);
		mDynamicLoad.showScaleConfig(false);
		mDynamicLoad.showMipMapSelection(false);
		mDynamicLoad.showOpacityConfig(false);
		mDynamicLoad.showShowTransparencyConfig(false);
		mDynamicLoad.showToneMappingConfig(false);
		mDynamicLoad.getTextureDesc().useTransparency = true;
	}

	TextureViewer::~TextureViewer() = default;

	void nex::gui::TextureViewer::drawSelf()
	{		
		static Future<Resource*> textureFuture;
		Texture* loadedTexture = nullptr;

		if (textureFuture.is_ready())
			loadedTexture = (Texture*)textureFuture.get();

		if (loadedTexture) {

			auto& desc = mDynamicLoad.getTextureDesc();
			desc.texture = loadedTexture;
			desc.flipY = nex::ImageFactory::isYFlipped();
			desc.sampler = Sampler::getDefaultImage();

			mDynamicLoad.updateTexture(true);
			mDynamicLoad.drawGUI();
		}


		if (ImGui::Button("Load Image")) {

			if (textureFuture.is_ready() || !textureFuture.valid()) {

				textureFuture = ResourceLoader::get()->enqueue([=]()->nex::Resource * {

					FileDialog fileDialog(mWindow);
					auto result = fileDialog.selectFile("jpg,png,psd,bpm,tga,hdr");

					TextureDesc data;
					data.colorspace = ColorSpace::SRGBA;
					data.internalFormat = InternalFormat::SRGBA8;
					data.generateMipMaps = true;

					Texture* texture = nullptr;

					switch (result.state) {
					case FileDialog::State::Okay:
						std::cout << "Selected file: " << result.path << std::endl;
						
						try {
							texture = TextureManager::get()->getImage(result.path.generic_u8string(), data, true);
						}
						catch (const std::exception & e) {
							LOG(Logger("Resource Loader: "), Error) << "Couldn't load texture: " << e.what();

							std::stringstream ss;
							ss << "Couldn't load image '" << result.path << "'. See log file for more information.";
							boxer::show(ss.str().c_str(), "Resource Loading Error", boxer::Style::Error, mWindow->getNativeWindow());
						}
						return texture;
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