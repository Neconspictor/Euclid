#include <nex/gui/TextureViewer.hpp>
#include <imgui/imgui.h>
#include <nex/gui/Util.hpp>
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


nex::gui::TextureViewer::TextureViewer(const glm::vec2& canvasSize, const std::string& title, nex::Window* window) :
	Drawable(),
	mTextureView({}, (const ImVec2&)canvasSize),
	mTexLoader(window),
	mTitle(title)
{
}

nex::gui::TextureViewer::~TextureViewer() = default;


const nex::Future<nex::Resource*>& nex::gui::TextureViewer::getTextureFuture() {
	return mFuture;
}

const nex::gui::TextureView& nex::gui::TextureViewer::getTextureView() const
{
	return mTextureView;
}

nex::gui::TextureView& nex::gui::TextureViewer::getTextureView()
{
	return mTextureView;
}

void nex::gui::TextureViewer::drawSelf()
{		
	Texture* loadedTexture = nullptr;

	if (mFuture.is_ready())
		loadedTexture = (Texture*)mFuture.get();

	if (loadedTexture) {

		auto& desc = mTextureView.getTextureDesc();
		desc.texture = loadedTexture;
		desc.flipY = nex::ImageFactory::isYFlipped();
		//desc.sampler = Sampler::getDefaultImage();

		mTextureView.updateTexture(true);
		mTextureView.drawGUI();
	}


	if (ImGui::Button(mTitle.c_str())) {

		if (mFuture.is_ready() || !mFuture.valid()) {
			mFuture = mTexLoader.selectTexture();
		}
	}     
}