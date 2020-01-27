#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/gui/ImGUI.hpp>
#include <nex/texture/Sampler.hpp>

namespace nex::gui
{
	class TextureView : public Drawable
	{
	public:
		TextureView(const ImGUI_TextureDesc& textureDesc, const glm::ivec2& viewSize);

		glm::ivec2 calcTextureSize(const ImGUI_TextureDesc& desc) const;

		ImGUI_TextureDesc& getTextureDesc();
		
		const glm::ivec2& getViewSize()const;

		void setScale(float scale);
		void setInterpretAsCubemap(bool interpret);

		/**
		 * Note: If the texture is negative, it is assumed that the original texture size should be used.
		 */
		void setTextureSize(const glm::ivec2& size = glm::ivec2(-1, -1));
		void setViewSize(const glm::ivec2& size);

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

		bool isNearestNeighborUsed() const;

		float getFitScale() const;

		/**
		 * Draws the GUI of this Drawable.
		 */
		void drawSelf() override;
		ImGUI_TextureDesc mDesc;
		glm::ivec2 mViewSize;
		glm::vec2 mTextureOverwriteSize;
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