#include <nex/gui/FileDialog.hpp>
#include <nfd/nfd.h>
#include <nex/Window.hpp>

nex::gui::FileDialog::FileDialog(nex::Window * window) : mWindow(window)
{
}

nex::gui::FileDialog::FileSelection nex::gui::FileDialog::selectFile()
{
	FileSelection out;

	nfdchar_t *outPath = NULL;
	nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath, mWindow->getNativeWindow());

	switch (result) {
	case NFD_OKAY:
		out.state = State::Okay;
		out.path = std::string(outPath);
		free(outPath);
		break;
	case NFD_CANCEL:
		out.state = State::Cancled;
		break;
	case NFD_ERROR:
		out.state = State::Error;
		out.error = NFD_GetError();
		break;
	}

	return out;
}