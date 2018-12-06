#include <nex/SubSystemProvider.hpp>
#include <nex/opengl/window_system/glfw/WindowGLFW.hpp>

namespace nex
{
	class SubSystemProviderGLFW : public SubSystemProvider
	{
	public:

		Window* createWindow(Window::WindowStruct& desc) override;

		std::unique_ptr<nex::gui::ImGUI_Impl> createGUI(Window* window) override;

		static SubSystemProviderGLFW* get();

		bool init() override;

		bool isTerminated() const override;

		void pollEvents() override;

		void terminate() override;

		static void errorCallback(int error, const char* description);


	protected:
		bool m_isInitialized;
		nex::Logger m_logger;

	private:
		SubSystemProviderGLFW();

		std::list<WindowGLFW> windows;
	};
}