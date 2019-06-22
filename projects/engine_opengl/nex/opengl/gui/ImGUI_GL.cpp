#include <nex/opengl/gui/ImGUI_GL.hpp>

#include <nex/Window.hpp>
#include <nex/Input.hpp>
#include "nex/texture/Texture.hpp"
#include "nex/mesh/VertexArray.hpp"
#include <nex/mesh/VertexBuffer.hpp>
#include "nex/mesh/IndexBuffer.hpp"
#include <nex/mesh/VertexLayout.hpp>
#include "nex/shader/Pass.hpp"

#include "nex/renderer/RenderBackend.hpp"
#include "nex/texture/Sampler.hpp"

namespace nex::gui
{
	class ImGUI_GL::Drawer : public nex::Pass
	{
	public:
		Drawer(const ShaderFilePath& vertexShader, const ShaderFilePath& fragmentShader)
		{
			mShader = nex::Shader::create(vertexShader, fragmentShader);

			mTexture = mShader->createTextureUniform("Texture", UniformType::TEXTURE2D, 0);
			mAxis = { mShader->getUniformLocation("Axis"), nex::UniformType::VEC3 };
			mProjMtx = { mShader->getUniformLocation("ProjMtx"), nex::UniformType::MAT4 };

		}

		void setTexture(nex::Texture* texture, Sampler* sampler)
		{
			if (!sampler) sampler = &mSampler;
			mShader->setTexture(texture, sampler, mTexture.bindingSlot);
		}

		void setProjMtx(const glm::mat4& mat)
		{
			mShader->setMat4(mProjMtx.location, mat);
		}

		void setAxis(const glm::vec3& vec)
		{
			mShader->setVec3(mAxis.location, vec);
		}

	private:

		nex::UniformTex mTexture;
		nex::Uniform mAxis;
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

		mShaderTexture2D->bind();
		mShaderTexture2D->setProjMtx(ortho_projection);

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
			mVertexBuffer->fill(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), ShaderBuffer::UsageHint::STREAM_DRAW);

			// index buffer update
			const IndexElementType type = sizeof(ImDrawIdx) == 4 ? IndexElementType::BIT_32 : IndexElementType::BIT_16;
			mIndices->bind();
			mIndices->fill(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, type);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					bindTextureShader((ImGUI_ImageDesc*)pcmd->TextureId, ortho_projection);

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

	void ImGUI_GL::bindTextureShader(ImGUI_ImageDesc* desc, const glm::mat4& projection)
	{
		Drawer* drawer = mShaderTexture2D.get();

		auto* texture = desc->texture;

		if (auto* cubemap = dynamic_cast<CubeMap*>(desc->texture))
		{
			drawer = mShaderCubeMap.get();
		}

		const bool init = !drawer->isBound();
		drawer->bind();
		drawer->setTexture(desc->texture, nullptr);

		if (init)
		{
			drawer->setAxis(glm::vec3(1, 0, 0));
			drawer->setProjMtx(projection);
		}
	}

	bool ImGUI_GL::createDeviceObjects()
	{
		mShaderTexture2D = std::make_unique<Drawer>("imgui/imgui_draw_vs.glsl", "imgui/imgui_draw_fs.glsl");
		mShaderCubeMap = std::make_unique<Drawer>("imgui/imgui_draw_vs.glsl", "imgui/imgui_draw_cubemap_fs.glsl");
		mVertexBuffer = std::make_unique<VertexBuffer>();
		mIndices = std::make_unique<IndexBuffer>();


		VertexLayout layout;
		layout.push<float>(2); // Position
		layout.push<float>(2); // UV
		layout.push<unsigned char>(4); // Color

		// NOTE: In order to support multiple GL contexts we have to recreate the vertex array on each render request; 
		// For now we use only one context, so this solution is fine
		mVertexArray = std::make_unique<VertexArray>();
		mVertexArray->bind();
		mVertexArray->useBuffer(*mVertexBuffer, layout);
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
		TextureData desc;
		desc.colorspace = ColorSpace::RGBA;
		desc.internalFormat = InternFormat::RGBA8;
		desc.pixelDataType = PixelDataType::UBYTE;
		desc.minFilter = desc.magFilter = TextureFilter::Linear;

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
