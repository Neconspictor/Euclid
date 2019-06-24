#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/gui/ImGUI.hpp>

namespace nex::gui
{
	class TextureView : public Drawable
	{
	public:
		TextureView(const ImGUI_ImageDesc& textureDesc, const ImVec2& viewSize);

		ImGUI_ImageDesc& getTexture();
		void updateTexture(bool updateScaleWhenChanged);
		void updateScale();

		void setViewSize(const ImVec2& size);

		const ImVec2& getViewSize()const;
		const ImVec2& getTextureSize()const;

	protected:

		class CheckerboardPattern;

		void addCheckBoardPattern(const ImVec2& size);
		static ImVec2 calcTextureSize(const ImGUI_ImageDesc& desc);

		/**
		 * Draws the GUI of this Drawable.
		 */
		void drawSelf() override;
		ImGUI_ImageDesc mDesc;
		ImVec2 mViewSize;
		ImVec2 mTextureSize;
		float mOpacity;
		float mScale;
	};
}