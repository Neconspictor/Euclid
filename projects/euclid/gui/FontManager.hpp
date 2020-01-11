#pragma once
#include <filesystem>
#include <nex/gui/Drawable.hpp>


namespace nex::gui
{

	class ImGUI_Impl;

	class FontManager
	{
	public:

		FontManager(ImGUI_Impl* impl);

		void addFont(const std::filesystem::path& fontPath, size_t pixelSize);
		void setGlobalFontScale(float scale);
		float getGlobalFontScale() const;

		void updateFonts();
		
	private:

		struct FontDesc {
			std::filesystem::path path;
			size_t size;
		};

		std::vector<FontDesc> mFonts;
		float mGlobalScale;
		ImGUI_Impl* mGui;

		constexpr static size_t mContentBaseSize = 14;
		constexpr static size_t mHeading2BaseSize = 16;
		constexpr static size_t mHeadingBaseSize = 18;
	};


	class FontManager_View : public nex::gui::Drawable {
	public:
		FontManager_View(FontManager* fontManager);
	protected:

		void drawSelf() override;

		FontManager* mFontManager;
	};
}