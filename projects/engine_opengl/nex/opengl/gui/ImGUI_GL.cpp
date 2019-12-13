#include <nex/opengl/gui/ImGUI_GL.hpp>

#include <nex/platform/Window.hpp>
#include <nex/platform/Input.hpp>
#include "nex/texture/Texture.hpp"
#include "nex/mesh/VertexArray.hpp"
#include <nex/buffer/VertexBuffer.hpp>
#include "nex/buffer/IndexBuffer.hpp"
#include <nex/mesh/VertexLayout.hpp>
#include <nex/shader/Shader.hpp>

#include "nex/renderer/RenderBackend.hpp"
#include "nex/texture/Sampler.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/texture/TextureManager.hpp>

namespace nex::gui
{
	class ImGUI_GL::Drawer : public nex::Shader
	{
	public:
		Drawer(const ShaderFilePath& vertexShader, const ShaderFilePath& fragmentShader)
		{
			mProgram = nex::ShaderProgram::create(vertexShader, fragmentShader);

			mTexture = mProgram->createTextureUniform("Texture", UniformType::TEXTURE2D, 0);
			mIndex = { mProgram->getUniformLocation("Index"), nex::UniformType::FLOAT };
			mSide = { mProgram->getUniformLocation("Side"), nex::UniformType::UINT };
			mMipMapLevel = { mProgram->getUniformLocation("MipMapLevel"), nex::UniformType::INT };
			mProjMtx = { mProgram->getUniformLocation("ProjMtx"), nex::UniformType::MAT4 };
			mUseTransparency = { mProgram->getUniformLocation("UseTransparency"), nex::UniformType::INT };
			mTransformUV = { mProgram->getUniformLocation("transformUV"), nex::UniformType::MAT3 };
			mFlipY = { mProgram->getUniformLocation("FlipY"), nex::UniformType::MAT3 };
			mGammaCorrect = { mProgram->getUniformLocation("UseGammaCorrection"), nex::UniformType::INT };
			mToneMapping = { mProgram->getUniformLocation("UseToneMapping"), nex::UniformType::INT };
		}

		void setArrayIndex(float index)
		{
			mProgram->setFloat(mIndex.location, index);
		}

		void setTexture(const nex::Texture* texture, const Sampler* sampler)
		{
			if (!sampler) sampler = Sampler::getLinearMipMap();
			mProgram->setTexture(texture, sampler, mTexture.bindingSlot);
		}

		void setProjMtx(const glm::mat4& mat)
		{
			mProgram->setMat4(mProjMtx.location, mat);
		}

		void setFlipY(bool value)
		{
			mProgram->setInt(mFlipY.location, value);
		}

		void setTransformUV(const glm::mat3& mat)
		{
			mProgram->setMat3(mTransformUV.location, mat);
		}

		void setCubeMapSide(CubeMapSide side)
		{
			mProgram->setUInt(mSide.location, (unsigned)side);
		}

		void setMipMapLevel(int level)
		{
			mProgram->setInt(mMipMapLevel.location, level);
		}

		void setUseTransparency(bool value)
		{
			mProgram->setInt(mUseTransparency.location, value);
		}

		void setGammaCorrect(bool value)
		{
			mProgram->setInt(mGammaCorrect.location, value);
		}

		void setUseToneMapping(bool value)
		{
			mProgram->setInt(mToneMapping.location, value);
		}

	private:

		nex::UniformTex mTexture;
		nex::Uniform mIndex;
		nex::Uniform mSide;
		nex::Uniform mMipMapLevel;
		nex::Uniform mTransformUV;
		nex::Uniform mFlipY;
		nex::Uniform mUseTransparency;
		nex::Uniform mGammaCorrect;
		nex::Uniform mToneMapping;
		nex::Uniform mProjMtx;
	};

	ImGUI_GL::ImGUI_GL(nex::Window* window) :
		mWindow(window),
		m_logger("ImGUI_GL")
	{
		mWindow->activate();
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		std::fill(g_MouseJustPressed, g_MouseJustPressed + 3, false);

		init();
	}

	ImGUI_GL::~ImGUI_GL()
	{
		//ImGui_ImplGlfwGL3_InvalidateDeviceObjects  // TODO!!!!!!!!!!!!!!!!!!!!!!!!!!

		ImGui::DestroyContext();
	}

	void ImGUI_GL::newFrame(float frameTime)
	{
		if (!mFontTexture)
			createDeviceObjects();

		ImGuiIO& io = ImGui::GetIO();

		// Setup display size (every frame to accommodate for window resizing)
		const auto width = mWindow->getFrameBufferWidth();
		const auto height = mWindow->getFrameBufferHeight();
		const auto frameBufferW = mWindow->getFrameBufferWidth();
		const auto frameBufferH = mWindow->getFrameBufferHeight();
		

		const auto* input = mWindow->getInputDevice();

		io.DisplaySize = ImVec2((float)width, (float)height);
		io.DisplayFramebufferScale = ImVec2(width > 0 ? ((float)frameBufferW / width) : 0, height > 0 ? ((float)frameBufferH / height) : 0);

		// Setup time step
		io.DeltaTime = frameTime;

		// Setup inputs		
		if (mWindow->hasFocus())
		{
			// Set OS mouse position if requested (only used when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
			if (io.WantSetMousePos)
			{
				mWindow->setCursorPosition(io.MousePos.x, io.MousePos.y);
			}
			else
			{
				const auto& mouseData = mWindow->getInputDevice()->getFrameMouseOffset();
				io.MousePos = ImVec2((float)mouseData.xAbsolute, (float)mouseData.yAbsolute);
			}
		}
		else
		{
			io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
		}

		for (int i = Input::BUTTON_MIN_VALUE; i < Input::BUTTON_SIZE; i++)
		{
			// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
			const auto button = static_cast<Input::Button>(Input::BUTTON_MIN_VALUE + i);
			io.MouseDown[i] = g_MouseJustPressed[i] || input->isDown(button);
			g_MouseJustPressed[i] = false;
		}

		//TODO use input class
		// Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
		if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0 && mWindow->getCursorState() != CursorState::Disabled)
		{
			ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
			if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
			{
				mWindow->showCursor(CursorState::Hidden);
			}
			else
			{
				mWindow->setCursor(mMouseCursors[cursor].get() ? mMouseCursors[cursor].get() : mMouseCursors[ImGuiMouseCursor_Arrow].get());
				mWindow->showCursor(CursorState::Normal);
			}
		}

		// Gamepad navigation mapping [BETA]
		memset(io.NavInputs, 0, sizeof(io.NavInputs));
		if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)
		{
			// For now we don't support gamepads
			io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
		}

		// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
		ImGui::NewFrame();
	}

	void ImGUI_GL::renderDrawData(ImDrawData * draw_data)
	{

		auto* backend = RenderBackend::get();
		auto* blender = backend->getBlender();
		auto* rasterizer = backend->getRasterizer();
		auto* depthTest = backend->getDepthBuffer();

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		ImGuiIO& io = ImGui::GetIO();
		int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
		int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fb_width == 0 || fb_height == 0)
			return;
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);

		// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
		//blender->enableBlend(true);
		BlendDesc desc;
		desc.operation = BlendOperation::ADD;
		desc.source = BlendFunc::SOURCE_ALPHA;
		desc.destination = BlendFunc::ONE_MINUS_SOURCE_ALPHA;
		//blender->setBlendDesc(desc);
		
		//rasterizer->enableFaceCulling(false);
		//depthTest->enableDepthTest(false);
		rasterizer->enableScissorTest(true);
		//rasterizer->setFillMode(FillMode::FILL);

		RenderState state;
		state.doDepthTest = false;
		state.doCullFaces = false;
		state.fillMode = FillMode::FILL;
		state.blendDesc = desc;
		state.doBlend = true;
		

		// Setup viewport, orthographic projection matrix
		RenderBackend::get()->setViewPort(0, 0, fb_width, fb_height);
		
		const glm::mat4 ortho_projection =
		{
			{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
			{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
			{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
			{ -1.0f,                  1.0f,                   0.0f, 1.0f },
		};

		mShaderGeneral->bind();
		mShaderGeneral->setProjMtx(ortho_projection);
		mShaderGeneral->setTransformUV(glm::mat3(1.0f));
		mShaderGeneral->setFlipY(false);

		// We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
		Sampler::unbind(0);

		mVertexArray->bind();

		// Draw
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			size_t idx_buffer_offset = 0;

			// vertex buffer update
			mVertexBuffer->bind();
			mVertexBuffer->resize(cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), cmd_list->VtxBuffer.Data, ShaderBuffer::UsageHint::STREAM_DRAW);

			// index buffer update
			const IndexElementType type = sizeof(ImDrawIdx) == 4 ? IndexElementType::BIT_32 : IndexElementType::BIT_16;
			mIndices->bind();
			mIndices->fill(type, cmd_list->IdxBuffer.Size, cmd_list->IdxBuffer.Data);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					mShaderGeneral->bind();
					pcmd->UserCallback(cmd_list, pcmd);
				} else
				{
					bindTextureShader((ImGUI_TextureDesc*)pcmd->TextureId, ortho_projection);
				}

				if (!pcmd->UserCallback || (pcmd->UserCallback && pcmd->ElemCount > 0))
				{
					backend->setScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
					backend->drawWithIndices(state, Topology::TRIANGLES, pcmd->ElemCount, mIndices->getType(), idx_buffer_offset);
				}

				idx_buffer_offset += pcmd->ElemCount * sizeof(ImDrawIdx);
			}
		}

		// Restore modified pipeline state
		blender->enableBlend(false);
		rasterizer->enableFaceCulling(false);
		depthTest->enableDepthTest(true);
		rasterizer->enableScissorTest(false);
		//rasterizer->setFillMode(FillMode::FILL, PolygonSide::FRONT);
	}

	void ImGUI_GL::shutdown() {}

	void ImGUI_GL::init()
	{
		// Setup back-end capabilities flags
		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;   // We can honor GetMouseCursor() values (optional)
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;    // We can honor io.WantSetMousePos requests (optional, rarely used)

																// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
		io.KeyMap[ImGuiKey_Tab] = Input::Key::KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = Input::Key::KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = Input::Key::KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = Input::Key::KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = Input::Key::KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = Input::Key::KEY_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown] = Input::Key::KEY_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home] = Input::Key::KEY_HOME;
		io.KeyMap[ImGuiKey_End] = Input::Key::KEY_END;
		io.KeyMap[ImGuiKey_Insert] = Input::Key::KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = Input::Key::KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = Input::Key::KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = Input::Key::KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = Input::Key::KEY_RETURN;
		io.KeyMap[ImGuiKey_Escape] = Input::Key::KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = Input::Key::KEY_A;
		io.KeyMap[ImGuiKey_C] = Input::Key::KEY_C;
		io.KeyMap[ImGuiKey_V] = Input::Key::KEY_V;
		io.KeyMap[ImGuiKey_X] = Input::Key::KEY_X;
		io.KeyMap[ImGuiKey_Y] = Input::Key::KEY_Y;
		io.KeyMap[ImGuiKey_Z] = Input::Key::KEY_Z;

		io.SetClipboardTextFn = ImGUI_GL::setClipboardText;
		io.GetClipboardTextFn = ImGUI_GL::getClipboardText;
		io.ClipboardUserData = mWindow->getInputDevice();
#ifdef _WIN32
		io.ImeWindowHandle = mWindow->getNativeWindow();
#endif

		// Load cursors
		// FIXME: GLFW doesn't expose suitable cursors for ResizeAll, ResizeNESW, ResizeNWSE. We revert to arrow cursor for those.
		mMouseCursors[ImGuiMouseCursor_Arrow] = std::make_unique<Cursor>(StandardCursorType::Arrow);
		mMouseCursors[ImGuiMouseCursor_TextInput] = std::make_unique<Cursor>(StandardCursorType::TextIBeam);
		mMouseCursors[ImGuiMouseCursor_ResizeAll] = std::make_unique<Cursor>(StandardCursorType::Arrow);
		mMouseCursors[ImGuiMouseCursor_ResizeNS] = std::make_unique<Cursor>(StandardCursorType::VerticalResize);
		mMouseCursors[ImGuiMouseCursor_ResizeEW] = std::make_unique<Cursor>(StandardCursorType::HorizontalResize);
		mMouseCursors[ImGuiMouseCursor_ResizeNESW] = std::make_unique<Cursor>(StandardCursorType::Arrow);
		mMouseCursors[ImGuiMouseCursor_ResizeNWSE] = std::make_unique<Cursor>(StandardCursorType::Arrow);

		auto* input = mWindow->getInputDevice();

		input->addMouseCallback([&](Input::Button button, Input::InputItemState state, int mods)
		{
			if (state == Input::Pressed)
				this->g_MouseJustPressed[button] = true;
		});

		input->addScrollCallback([](double scrollX, double scrollY) {
			//std::cout << "scroll callback: scrollX: " << scrollX << ", scrollY: " << scrollY << std::endl;
			ImGuiIO& io = ImGui::GetIO();
			io.MouseWheelH += (float)scrollX;
			io.MouseWheel += (float)scrollY;
		});

		input->addCharCallback([&](unsigned int codepoint, int mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (codepoint > 0 && codepoint < 0x10000)
				io.AddInputCharacter((unsigned short)codepoint);
		});

		input->addKeyCallback([&](Input::Key key, Input::InputItemState state, int scancode, int mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.KeysDown[key] = false;
			if (state == Input::Down || state == Input::Pressed)
				io.KeysDown[key] = true;

			(void)mods; // Modifiers are not reliable across systems
			io.KeyCtrl = io.KeysDown[Input::Key::KEY_LEFT_CONTROL] || io.KeysDown[Input::KEY_RIGHT_CONTROL];
			io.KeyShift = io.KeysDown[Input::Key::KEY_LEFT_SHIFT] || io.KeysDown[Input::Key::KEY_RIGHT_SHIFT];
			io.KeyAlt = io.KeysDown[Input::Key::KEY_LEFT_ALT] || io.KeysDown[Input::Key::KEY_RIGHT_ALT];
			io.KeySuper = io.KeysDown[Input::Key::KEY_LEFT_SUPER] || io.KeysDown[Input::Key::KEY_RIGHT_SUPER];
		});


		ImGui::StyleColorsDark();

		createDeviceObjects();
	}

	void ImGUI_GL::bindTextureShader(ImGUI_TextureDesc* desc, const glm::mat4& proj)
	{
		Drawer* drawer = mShaderTexture2D.get();

		static const glm::mat3 flipY =
		{
			{ 1.0f, 0.0f, 0.0f},
			{ 0.0f, -1.0, 0.0f},
			{ 0.0f, 1.0f, 1.0f}
		};

		static const glm::mat3 noFlipY(1.0f);

		auto* texture = desc->texture;

		if (!texture)
			throw_with_trace(std::invalid_argument("ImGUI_GL::bindTextureShader : Texture mustn't be null!"));

		const auto& pixelDataType = texture->getTextureData().pixelDataType;
		const auto target = texture->getTarget();

		if (target == TextureTarget::CUBE_MAP)
		{
			drawer = mShaderCubeMap.get();
		}
		else if (target == TextureTarget::CUBE_MAP_ARRAY) {
			drawer = mShaderCubeMapArray.get();
		}
		else if (target == TextureTarget::TEXTURE2D_ARRAY) {
			drawer = mShaderTexture2DArray.get();
		}

		const bool init = !drawer->isBound();
		drawer->bind();
		drawer->setTexture(desc->texture, desc->sampler);
		drawer->setArrayIndex(desc->level);
		drawer->setCubeMapSide(desc->side);
		drawer->setMipMapLevel(desc->lod);
		drawer->setUseTransparency(desc->useTransparency);

		auto colorspace = texture->getTextureData().colorspace;
		drawer->setGammaCorrect(true);

		drawer->setUseToneMapping(desc->useToneMapping);

		if (init) {
			auto& io = ImGui::GetIO();
			drawer->setProjMtx(proj);
		}

		if (desc->flipY) {
			auto& io = ImGui::GetIO();
			drawer->setTransformUV(flipY);
		}
		else {
			auto& io = ImGui::GetIO();
			drawer->setTransformUV(noFlipY);
		}
	}

	bool ImGUI_GL::createDeviceObjects()
	{
		mShaderGeneral = std::make_unique<Drawer>("imgui/imgui_draw_vs.glsl", "imgui/imgui_draw_fs.glsl");
		mShaderTexture2D = std::make_unique<Drawer>("imgui/imgui_draw_vs.glsl", "imgui/imgui_draw_fs.glsl");
		mShaderTexture2DArray = std::make_unique<Drawer>("imgui/imgui_draw_vs.glsl", "imgui/imgui_draw_texture2d_array_fs.glsl");
		mShaderCubeMap = std::make_unique<Drawer>("imgui/imgui_draw_vs.glsl", "imgui/imgui_draw_cubemap_fs.glsl");
		mShaderCubeMapArray = std::make_unique<Drawer>("imgui/imgui_draw_vs.glsl", "imgui/imgui_draw_cubemap_array_fs.glsl");
		mVertexBuffer = std::make_unique<VertexBuffer>();
		mIndices = std::make_unique<IndexBuffer>();


		VertexLayout layout;
		layout.push<float>(2, mVertexBuffer.get(), false, false, true); // Position
		layout.push<float>(2, mVertexBuffer.get(), false, false, true); // UV
		layout.push<unsigned char>(4, mVertexBuffer.get(), true, false, true); // Color

		// NOTE: In order to support multiple GL contexts we have to recreate the vertex array on each render request; 
		// For now we use only one context, so this solution is fine
		mVertexArray = std::make_unique<VertexArray>();
		mVertexArray->setLayout(layout);
		mVertexArray->init();
		mVertexArray->unbind();

		createFontsTexture();

		return true;
	}

	void ImGUI_GL::createFontsTexture()
	{
		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

																  // Upload texture to graphics system
		TextureDesc desc;
		desc.colorspace = ColorSpace::RGBA;
		desc.internalFormat = InternalFormat::RGBA8;
		desc.pixelDataType = PixelDataType::UBYTE;
		desc.minFilter = desc.magFilter = TexFilter::Linear;

		mFontTexture = std::make_unique<Texture2D>(width, height, desc, pixels);
		mFontDesc.texture = mFontTexture.get();
		mFontDesc.level = 0;
		mFontDesc.lod = 0;
		mFontDesc.sampler = nullptr;

		// Store our identifier
		io.Fonts->TexID = &mFontDesc;
	}

	const char* ImGUI_GL::getClipboardText(void* inputDevice)
	{
		return ((Input*)inputDevice)->getClipBoardText();
	}

	void ImGUI_GL::setClipboardText(void* inputDevice, const char* text)
	{
		((Input*)inputDevice)->setClipBoardText(text);
	}
}
