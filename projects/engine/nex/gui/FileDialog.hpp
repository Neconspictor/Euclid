#pragma once

#include <nex/gui/Drawable.hpp>

namespace nex {
	class Window;
}

namespace nex::gui
{

	class FileDialog {
	public:

		enum class State {
			Okay,
			Cancled,
			Error
		};

		struct FileSelection {
			std::filesystem::path path;
			std::string error;
			State state;
		};

		FileDialog(nex::Window* window = nullptr);

		/**
		 * Opens a file dialog for the user.
		 * Note: Is blocking.
		 */
		FileSelection selectFile();


	private:
		nex::Window* mWindow;

	};
}