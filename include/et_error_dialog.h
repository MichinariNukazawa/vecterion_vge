/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
/*! 	@file
 *	@brief color struct and utilitys.
 *
 * 	@attention convert methods is hard coding Magick Number.
 */
#ifndef include_ET_ERROR_DIALOG_H
#define include_ET_ERROR_DIALOG_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include <stdint.h>

#define show_error_dialog(parent_window, fmt, ...) \
	do{ \
		et_error(""fmt"", ## __VA_ARGS__); \
		char *message = g_strdup_printf(""fmt"", ## __VA_ARGS__ ); \
		show_error_dialog_impl_(parent_window, message); \
		g_free(message); \
	}while(0)

static inline void show_error_dialog_impl_(GtkWindow *parent_window, const char *message)
{
	et_assert(message);

	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *dialog = gtk_message_dialog_new (
			parent_window,
			flags,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			"%s",
			message);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	return;
}

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_ERROR_DIALOG_H

