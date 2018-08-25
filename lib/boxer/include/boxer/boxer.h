/*The MIT License(MIT)

Copyright(c) 2014 Aaron Jacobs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * NOTE: This file was modified to meet the needs of NeX
 */

#ifndef BOXER_H
#define BOXER_H

#if defined(BOXER_DLL) && defined(BOXER_BUILD_DLL)
   /*!
    * BOXER_DLL must be defined by applications that are linking against the DLL version of the Boxer library.
    * BOXER_BUILD_DLL is defined when compiling the DLL version of the library.
    */
   #error "You may not have both BOXER_DLL and BOXER_BUILD_DLL defined"
#endif

/*!
 * BOXERAPI is used to declare public API classes / functions for export from the DLL / shared library / dynamic library
 */
#if defined(_WIN32) && defined(BOXER_BUILD_DLL)
   // We are building Boxer as a Win32 DLL
   #define BOXERAPI __declspec(dllexport)
#elif defined(_WIN32) && defined(BOXER_DLL)
   // We are calling Boxer as a Win32 DLL
   #define BOXERAPI __declspec(dllimport)
#elif defined(__GNUC__) && defined(BOXER_BUILD_DLL)
   // We are building Boxer as a shared / dynamic library
   #define BOXERAPI __attribute__((visibility("default")))
#else
   // We are building or calling Boxer as a static library
   #define BOXERAPI
#endif

namespace boxer {

/*!
 * Options for styles to apply to a message box
 */
enum class Style {
   Info,
   Warning,
   Error,
   Question
};

/*!
 * Options for buttons to provide on a message box
 */
enum class Buttons {
   OK,
   OKCancel,
   YesNo,
   Quit
};

/*!
 * Possible responses from a message box. 'None' signifies that no option was chosen, and 'Error' signifies that an
 * error was encountered while creating the message box.
 */
enum class Selection {
   OK,
   Cancel,
   Yes,
   No,
   Quit,
   None,
   Error
};

/*!
 * The default style to apply to a message box
 */
const Style kDefaultStyle = Style::Info;

/*!
 * The default buttons to provide on a message box
 */
const Buttons kDefaultButtons = Buttons::OK;

/*!
 * Blocking call to create a modal message box with the given message, title, style, and buttons
 */
BOXERAPI Selection show(const char *message, const char *title, Style style, Buttons buttons, void* nativeWindow = nullptr);

/*!
 * Convenience function to call show() with the default buttons
 */
inline Selection show(const char *message, const char *title, Style style, void* nativeWindow = nullptr) {
   return show(message, title, style, kDefaultButtons, nativeWindow);
}

/*!
 * Convenience function to call show() with the default style
 */
inline Selection show(const char *message, const char *title, Buttons buttons, void* nativeWindow = nullptr) {
   return show(message, title, kDefaultStyle, buttons, nativeWindow);
}

/*!
 * Convenience function to call show() with the default style and buttons
 */
inline Selection show(const char *message, const char *title, void* nativeWindow = nullptr) {
   return show(message, title, kDefaultStyle, kDefaultButtons, nativeWindow);
}

} // namespace boxer

#endif