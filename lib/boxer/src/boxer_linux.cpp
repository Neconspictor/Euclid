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
 
#include <boxer/boxer.h>
#include <gtk/gtk.h>

namespace boxer {

namespace {

GtkMessageType getMessageType(Style style) {
   switch (style) {
      case Style::Info:
         return GTK_MESSAGE_INFO;
      case Style::Warning:
         return GTK_MESSAGE_WARNING;
      case Style::Error:
         return GTK_MESSAGE_ERROR;
      case Style::Question:
         return GTK_MESSAGE_QUESTION;
      default:
         return GTK_MESSAGE_INFO;
   }
}

GtkButtonsType getButtonsType(Buttons buttons) {
   switch (buttons) {
      case Buttons::OK:
         return GTK_BUTTONS_OK;
      case Buttons::OKCancel:
         return GTK_BUTTONS_OK_CANCEL;
      case Buttons::YesNo:
         return GTK_BUTTONS_YES_NO;
     case Buttons::Quit:
         return GTK_BUTTONS_CLOSE;
      default:
         return GTK_BUTTONS_OK;
   }
}

Selection getSelection(gint response) {
   switch (response) {
      case GTK_RESPONSE_OK:
         return Selection::OK;
      case GTK_RESPONSE_CANCEL:
         return Selection::Cancel;
      case GTK_RESPONSE_YES:
         return Selection::Yes;
      case GTK_RESPONSE_NO:
         return Selection::No;
      case GTK_RESPONSE_CLOSE:
         return Selection::Quit;
      default:
         return Selection::None;
   }
}

} // namespace

Selection show(const char *message, const char *title, Style style, Buttons buttons, void* nativeWindow) {
   if (!gtk_init_check(0, nullptr)) {
      return Selection::Error;
   }

   // Create a parent window to stop gtk_dialog_run from complaining
   GtkWidget *parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);

   GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                              GTK_DIALOG_MODAL,
                                              getMessageType(style),
                                              getButtonsType(buttons),
                                              "%s",
                                              message);
   gtk_window_set_title(GTK_WINDOW(dialog), title);
   Selection selection = getSelection(gtk_dialog_run(GTK_DIALOG(dialog)));

   gtk_widget_destroy(GTK_WIDGET(dialog));
   gtk_widget_destroy(GTK_WIDGET(parent));
   while (g_main_context_iteration(nullptr, false));

   return selection;
}

} // namespace boxer
