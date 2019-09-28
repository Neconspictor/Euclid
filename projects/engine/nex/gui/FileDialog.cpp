#include <nex/gui/FileDialog.hpp>
#include <nfd/nfd.h>
#include <nex/Window.hpp>
#include <filesystem>

nex::gui::FileDialog::FileDialog(nex::Window * window) : mWindow(window)
{
}

nex::gui::FileDialog::FileSelection nex::gui::FileDialog::selectFile(const char* filterList, const char* defaultPath)
{
	FileSelection out;

	nfdchar_t *outPath = NULL;
	nfdresult_t result = NFD_OpenDialog(filterList, defaultPath, &outPath, mWindow->getNativeWindow());

	switch (result) {
	case NFD_OKAY:
		out.state = State::Okay;
		out.path = std::filesystem::u8path(outPath);
		break;
	case NFD_CANCEL:
		out.state = State::Cancled;
		break;
	case NFD_ERROR:
		out.state = State::Error;
		out.error = NFD_GetError();
		break;
	}

	if (outPath)free(outPath);

	return out;
}