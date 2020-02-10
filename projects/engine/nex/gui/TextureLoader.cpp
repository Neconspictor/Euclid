#include <nex/gui/TextureLoader.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/TextureManager.hpp>
#include <boxer/boxer.h>
#include <nex/platform/Window.hpp>



nex::gui::TextureLoader::TextureLoader(nex::Window* window) : mWindow(window)
{
}

nex::Future<nex::Resource*> nex::gui::TextureLoader::selectTexture()
{
	return nex::ResourceLoader::get()->enqueue([=]()->nex::Resource* {

		using namespace nex::gui;

		FileDialog fileDialog(mWindow);
		auto result = fileDialog.selectFile("jpg,png,psd,bpm,tga,hdr");

		TextureDesc data;
		data.internalFormat = InternalFormat::SRGBA8;
	
		data.generateMipMaps = true;

		Texture* texture = nullptr;

		switch (result.state) {
		case FileDialog::State::Okay:
			std::cout << "Selected file: " << result.path << std::endl;

			try {
				auto extension = result.path.extension().generic_string();
				std::transform(extension.begin(), extension.end(), extension.begin(), std::tolower);
				if (extension == ".hdr") {
					data.internalFormat = InternalFormat::RGBA32F;
				}

				texture = TextureManager::get()->getImage(result.path, true, data, true);
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