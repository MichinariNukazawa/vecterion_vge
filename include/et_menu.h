/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_MENU_H
#define include_ET_MENU_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "et_doc_id.h"
#include "et_state.h"
#include "pv_image_file_read_option.h"

#define VECTERION_FULLNAME "Vecterion Vector Graphic Editor"

typedef struct{
	GtkWindow *window;
	GtkWidget *status_bar;
	GtkBuilder *document_new_dialog_builder;
	GtkBuilder *document_preference_dialog_builder;
	const char *vecterion_build;
}EtWindow;

bool gui_quit_();
bool output_file_from_doc_id_(const char *filepath, EtDocId doc_id);
EtDocId open_doc_new_from_file_(
		const char* filepath,
		const PvImageFileReadOption *imageFileReadOption);

bool init_menu(EtWindow *window, GtkWidget *box_root);

void slot_et_menu_from_etaion_change_state(EtState state, gpointer data);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_MENU_H

