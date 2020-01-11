#include <gui/FontManager.hpp>
#include <nex/gui/ImGUI.hpp>
#include <nex/renderer/RenderEngine.hpp>

namespace nex::gui
{
	FontManager::FontManager(ImGUI_Impl* impl) : mGui(impl), mGlobalScale(1.0f)
	{
		mGui->setContentFontSize(static_cast<size_t>(mContentBaseSize * mGlobalScale));
		mGui->setHeading2FontSize(static_cast<size_t>(mHeading2BaseSize * mGlobalScale));
		mGui->setHeadingFontSize(static_cast<size_t>(mHeadingBaseSize * mGlobalScale));

		const auto& defaultFontFamily = mGui->getDefaultFontFamily();

		mFonts.push_back({ defaultFontFamily , mContentBaseSize}); // Important: Has to be first item (-> default font)
		mFonts.push_back({ defaultFontFamily , mHeading2BaseSize });
		mFonts.push_back({ defaultFontFamily , mHeadingBaseSize });
	}

	void FontManager::addFont(const std::filesystem::path& fontPath, size_t pixelSize)
	{
		mFonts.push_back({fontPath, pixelSize});
	}
	
	void FontManager::setGlobalFontScale(float scale)
	{
		mGlobalScale = scale;
		auto content = std::max<size_t>(static_cast<size_t>(mContentBaseSize * mGlobalScale),1);
		auto heading2 = std::max<size_t>(static_cast<size_t>(mHeading2BaseSize * mGlobalScale), 1);
		auto heading = std::max<size_t>(static_cast<size_t>(mHeadingBaseSize * mGlobalScale), 1);
		mGui->setContentFontSize(content);
		mGui->setHeading2FontSize(heading2);
		mGui->setHeadingFontSize(heading);

		updateFonts();
	}

	void FontManager::updateFonts()
	{
		mGui->clearFonts();

		for (const auto& desc : mFonts) {

			auto pixelSize = std::max<size_t>(static_cast<size_t>(desc.size * mGlobalScale), 1);
			mGui->loadFont(desc.path, pixelSize);
		}

		mGui->updateFontsTexture();
	}

	float FontManager::getGlobalFontScale() const
	{
		return mGlobalScale;
	}
	FontManager_View::FontManager_View(FontManager* fontManager) : mFontManager(fontManager)
	{
	}
	void FontManager_View::drawSelf()
	{

		auto scale = mFontManager->getGlobalFontScale();
		if (ImGui::DragFloat("Text scale", &scale, 0.01f)) {
			scale = std::max(scale, 0.0f);

			nex::RenderEngine::getCommandQueue()->push([fontManager = mFontManager, scale = scale]() {
				fontManager->setGlobalFontScale(scale);
			});
		}
	}
}