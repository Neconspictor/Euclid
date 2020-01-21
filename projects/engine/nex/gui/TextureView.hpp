#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/gui/ImGUI.hpp>
#include <nex/texture/Sampler.hpp>

namespace nex::gui
{
	class TextureView : public Drawable
	{
	public:
		TextureView(const ImGUI_TextureDesc& textureDesc, const ImVec2& viewSize);

		ImGUI_TextureDesc& getTextureDesc();
		void updateTexture(bool updateScaleWhenChanged);
		void updateScale();

		void setInterpretAsCubemap(bool interpret);
		void setViewSize(const ImVec2& size);

		const ImVec2& getViewSize()const;
		const ImVec2& getTextureSize()const;

		void overwriteTextureSize(bool overwrite, const ImVec2& size);

		void showAllOptions(bool show);
		void showMipMapSelection(bool show);
		void showScaleConfig(bool show);
		void showOpacityConfig(bool show);
		void showShowTransparencyConfig(bool show);
		void showToneMappingConfig(bool show);
		void showFilteringConfig(bool show);

		void useNearestNeighborFiltering();
		void useLinearFiltering();

	protected:

		class CheckerboardPattern;

		void addCheckBoardPattern(const ImVec2& size);
		static ImVec2 calcTextureSize(const ImGUI_TextureDesc& desc);

		bool isNearestNeighborUsed() const;

		/**
		 * Draws the GUI of this Drawable.
		 */
		void drawSelf() override;
		ImGUI_TextureDesc mDesc;
		ImVec2 mViewSize;
		ImVec2 mTextureSize;
		float mScale;
		float mOpacity;
		std::string mScrollPaneID;
		Sampler mSampler;
		int mSelectedFiltering;

		bool mInterpretAsCubeMap = false;
		bool mTextureSizeIsOverwritten = false;

		//view configuation
		bool mShowMipMapSelection;
		bool mShowScaleConfig;
		bool mShowOpacityConfig;
		bool mShowShowTransparencyConfig;
		bool mShowToneMappingConfig;
		bool mShowFilteringConfig;
	};
}