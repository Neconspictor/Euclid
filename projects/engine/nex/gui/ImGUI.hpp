#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_internal.h>
#include <nex/math/Constant.hpp>
#include <nex/texture/TextureSamplerData.hpp>
#include <nex/common/Log.hpp>
#include <nex/resource/FileSystem.hpp>

namespace nex
{
	class Cursor;
	class IndexBuffer;
	class Sampler;
	class Texture;
	class Texture2D;
	class VertexArray;
	class VertexBuffer;
	class Window;
}

namespace nex::gui
{

	struct ImGUI_TextureDesc
	{
		// the texture to display - mustn't be null!
		const Texture* texture = nullptr;

		// can be nullptr - then a default sampler object is used
		Sampler* sampler = nullptr;

		// level of Detail (mipmap); only used if the sampler's minfilter supports mipmapping.
		unsigned lod = 0;
		// Only for texture arrays, 3d textures or cubemap arrays. 
		// Specify the index for a texture array, the depth for a 3d texture or the side of a cubemap.
		unsigned level = 0;

		// Only used for cubemaps and cubemap arrays
		CubeMapSide side = CubeMapSide::POSITIVE_X;

		bool useTransparency = true;
		bool useToneMapping = false;
		bool flipY = false;
	};

	/**
	 * Defines Rendering backend of imgui
	 * https://github.com/ocornut/imgui
	 */
	class ImGUI_Impl
	{
	public:
		virtual ~ImGUI_Impl();
		ImGUI_Impl(const ImGUI_Impl&) = delete;
		ImGUI_Impl& operator=(const ImGUI_Impl&) = delete;

		ImGUI_Impl(ImGUI_Impl&&) = default;
		ImGUI_Impl& operator=(ImGUI_Impl&&) = default;

		void newFrame(float frameTime);

		void init(nex::Window* window, const std::filesystem::path& fontRootPath);
		void release();

		static ImGUI_Impl* get();

		static bool isActive();

		void renderDrawData(ImDrawData* draw_data);


		void clearFonts();
		ImFont* getFont(const std::filesystem::path& fontPath, size_t pixelSize);
		ImFont* loadFont(const std::filesystem::path& fontPath, size_t pixelSize);


		void updateFontsTexture();

		void setDefaultFontFamily(const std::filesystem::path& fontPath);
		ImFont* getDefaultFont(size_t pixelSize, bool useFallback = true);
		const std::filesystem::path& getDefaultFontFamily() const;

		void setHeadingFontSize(size_t pixelSize);		// for big headings
		void setHeading2FontSize(size_t pixelSize);		// smaller headings
		void setContentFontSize(size_t pixelSize);		// normal text 

		size_t getHeadingFontSize() const;
		size_t getHeading2FontSize() const;
		size_t getContentFontSize() const;


	protected:

		ImGUI_Impl();

		static const char* getClipboardText(void* inputDevice);
		static void setClipboardText(void* inputDevice, const char* text);

		void bindTextureShader(ImGUI_TextureDesc* texture, const glm::mat4& proj);

		bool createDeviceObjects();

	protected:

		class Drawer;

		nex::Window* mWindow;
		std::unique_ptr<Cursor> mMouseCursors[ImGuiMouseCursor_COUNT];
		bool         g_MouseJustPressed[3];
		std::unique_ptr<Texture2D> mFontTexture;
		ImGUI_TextureDesc mFontDesc;
		//GLuint g_VboHandle;
		std::unique_ptr<VertexArray> mVertexArray;
		std::unique_ptr<VertexBuffer> mVertexBuffer;
		std::unique_ptr<IndexBuffer> mIndices;
		std::unique_ptr<Drawer> mShaderGeneral;
		std::unique_ptr<Drawer> mShaderTexture2D;
		std::unique_ptr<Drawer> mShaderTexture2DArray;
		std::unique_ptr<Drawer> mShaderCubeMap;
		std::unique_ptr<Drawer> mShaderCubeMapArray;
		nex::Logger mLogger;

		struct FontID {
			std::filesystem::path fontPath;
			size_t pixelSize;

			bool operator<(const FontID& b) const;
		};

		std::map<FontID, ImFont*> mFonts;
		std::unique_ptr<FileSystem> mFileSystem;
		std::filesystem::path mDefaultFontFamily;

		size_t mHeadingFontSize = 18;
		size_t mHeading2FontSize = 16;
		size_t mContentFontSize = 14;
	};
}