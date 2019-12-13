#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <nex/gui/TextureView.hpp>
#include <nex/common/Future.hpp>
#include <nex/gui/TextureLoader.hpp>


namespace nex
{
	class Scene;
	class Window;
	class Vob;
	class Resource;
}

namespace nex::gui
{
	class TextureViewer : public nex::gui::Drawable
	{
	public:
		TextureViewer(const glm::vec2& canvasSize, const std::string& title, nex::Window* widow);
		virtual ~TextureViewer();

		const TextureView& getTextureView() const;
		TextureView& getTextureView();

	protected:

		void drawSelf() override;
        
		TextureView mTextureView;
		nex::Future<Resource*> mFuture;
		TextureLoader mTexLoader;
		std::string mTitle;
	};
}