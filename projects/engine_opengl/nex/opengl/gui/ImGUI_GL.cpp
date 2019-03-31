#include <nex/opengl/gui/ImGUI_GL.hpp>
#include <nex/opengl/opengl.hpp>

#include <nex/Window.hpp>
#include <nex/Input.hpp>

namespace nex::gui
{
	ImGUI_GL::ImGUI_GL(nex::Window* window, std::string glsl_version) :
		mWindow(window),
		glsl_version(move(glsl_version)),
		g_FontTexture(GL_FALSE),
		g_ShaderHandle(0), g_VertHandle(0), g_FragHandle(0),
		g_AttribLocationTex(0), g_AttribLocationProjMtx(0),
		g_AttribLocationPosition(0), g_AttribLocationUV(0), g_AttribLocationColor(0),
		g_VboHandle(0), g_ElementsHandle(0),
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
		if (!g_FontTexture)
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
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		ImGuiIO& io = ImGui::GetIO();
		int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
		int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fb_width == 0 || fb_height == 0)
			return;
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);

		// Backup GL state
		GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
		glActiveTexture(GL_TEXTURE0);
		GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
		GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
		GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
		GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
		GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
		GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
		GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
		GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
		GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
		GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
		GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
		GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
		GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
		GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
		GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
		GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

		// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Setup viewport, orthographic projection matrix
		glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
		const float ortho_projection[4][4] =
		{
			{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
			{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
			{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
			{ -1.0f,                  1.0f,                   0.0f, 1.0f },
		};
		glUseProgram(g_ShaderHandle);
		glUniform1i(g_AttribLocationTex, 0);
		glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
		if (glBindSampler) glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.

												// Recreate the VAO every time 
												// (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts, and we don't track creation/deletion of windows so we don't have an obvious key to use to cache them.)
		GLuint vao_handle = 0;
		glGenVertexArrays(1, &vao_handle);
		glBindVertexArray(vao_handle);
		glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
		glEnableVertexAttribArray(g_AttribLocationPosition);
		glEnableVertexAttribArray(g_AttribLocationUV);
		glEnableVertexAttribArray(g_AttribLocationColor);
		glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
		glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

		// Draw
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			const ImDrawIdx* idx_buffer_offset = 0;

			glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
					glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
					glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
				}
				idx_buffer_offset += pcmd->ElemCount;
			}
		}
		glDeleteVertexArrays(1, &vao_handle);

		// Restore modified GL state
		glUseProgram(last_program);
		glBindTexture(GL_TEXTURE_2D, last_texture);
		if (glBindSampler) glBindSampler(0, last_sampler);
		glActiveTexture(last_active_texture);
		glBindVertexArray(last_vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
		glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
		if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
		if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
		glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
		glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
	}

	void ImGUI_GL::shutdown()
	{
	}

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
			if (state == Input::Down)
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

	bool ImGUI_GL::createDeviceObjects()
	{
		// Backup GL state
		GLint last_texture, last_array_buffer, last_vertex_array;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

		const GLchar* vertex_shader =
			"uniform mat4 ProjMtx;\n"
			"in vec2 Position;\n"
			"in vec2 UV;\n"
			"in vec4 Color;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"	Frag_UV = UV;\n"
			"	Frag_Color = Color;\n"
			"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* fragment_shader =
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
			"}\n";

		const GLchar* vertex_shader_with_version[2] = { "#version 150\n", vertex_shader };
		const GLchar* fragment_shader_with_version[2] = { "#version 150\n", fragment_shader };

		g_ShaderHandle = glCreateProgram();

		g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(g_VertHandle, 2, vertex_shader_with_version, NULL);
		glCompileShader(g_VertHandle);

		// check compilation
		GLint result = GL_FALSE;
		int logInfoLength;
		glGetShaderiv(g_VertHandle, GL_COMPILE_STATUS, &result);
		glGetShaderiv(g_VertHandle, GL_INFO_LOG_LENGTH, &logInfoLength);


		if (logInfoLength > 0)
		{
			std::vector<char> shaderErrorMessage(logInfoLength + 1);
			glGetShaderInfoLog(g_VertHandle, logInfoLength, nullptr, &shaderErrorMessage[0]);
			LOG(m_logger, Error) << &shaderErrorMessage[0];
		}

		g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(g_FragHandle, 2, fragment_shader_with_version, NULL);
		glCompileShader(g_FragHandle);

		glGetShaderiv(g_FragHandle, GL_COMPILE_STATUS, &result);
		glGetShaderiv(g_FragHandle, GL_INFO_LOG_LENGTH, &logInfoLength);

		if (logInfoLength > 0)
		{
			std::vector<char> shaderErrorMessage(logInfoLength + 1);
			glGetShaderInfoLog(g_FragHandle, logInfoLength, nullptr, &shaderErrorMessage[0]);
			LOG(m_logger, Error) << &shaderErrorMessage[0];
		}


		glAttachShader(g_ShaderHandle, g_VertHandle);
		glAttachShader(g_ShaderHandle, g_FragHandle);
		glLinkProgram(g_ShaderHandle);

		g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");

		g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
		g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
		g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
		g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

		glGenBuffers(1, &g_VboHandle);
		glGenBuffers(1, &g_ElementsHandle);

		createFontsTexture();

		// Restore modified GL state
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBindVertexArray(last_vertex_array);

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
		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGenTextures(1, &g_FontTexture);
		glBindTexture(GL_TEXTURE_2D, g_FontTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		// Store our identifier
		io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

		// Restore state
		glBindTexture(GL_TEXTURE_2D, last_texture);
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