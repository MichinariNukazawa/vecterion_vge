#include "et_doc_relation.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include <glib/gi18n.h>

#include "et_error.h"
#include "et_doc_manager.h"
#include "et_canvas_collection.h"
#include "et_layer_view.h"
#include "et_color_panel.h"
#include "et_stroke_panel.h"
#include "et_position_panel.h"
#include "et_snap_panel.h"
#include "et_etaion.h"

static bool _confirm_dialog_run(
		GtkWidget *window,
		const char *dialog_title,
		const char *message,
		const char *accept_button_title);

EtDocId open_doc_new(PvVg *vg_src)
{
	EtDocId doc_id = et_doc_manager_new_doc_from_vg(vg_src);
	if(0 > doc_id){
		et_error("");
		return -1;
	}

	// ** gui
	EtCanvas *canvas = et_canvas_collection_new_canvas(doc_id);
	if(NULL == canvas){
		et_error("");
		return -1;
	}

	if(!et_doc_add_slot_change(doc_id, et_layer_view_slot_from_doc_change, NULL)){
		et_error("");
		return -1;
	}

	if(!et_doc_add_slot_change(doc_id, slot_et_color_panel_from_doc_change, NULL)){
		et_error("");
		return -1;
	}

	if(!et_doc_add_slot_change(doc_id, slot_et_stroke_panel_from_doc_change, NULL)){
		et_error("");
		return -1;
	}

	if(!et_doc_add_slot_change(doc_id, slot_et_position_panel_from_doc_change, NULL)){
		et_error("");
		return -1;
	}

	if(!et_doc_add_slot_change(doc_id, slot_et_snap_panel_from_doc_change, NULL)){
		et_error("");
		return -1;
	}

	et_etaion_set_current_doc_id(doc_id);

	return doc_id;
}

void close_doc_from_id(EtDocId doc_id, GtkWidget *widget)
{
	if(doc_id < 0){
		et_bug("");
		return;
	}

	if(!et_doc_is_saved_from_id(doc_id)){
		if(NULL == widget){
			// skip to confirm
			et_debug("Close: skip to confirm.");
		}else{
			if(!_confirm_dialog_run(
						gtk_widget_get_toplevel(widget),
						"Close", "Close now?", "Close")){
				// Cancel
				return;
			}
		}
	}

	EtDocId next_doc_id = et_canvas_collection_get_other_doc_id(doc_id);

	et_etaion_set_current_doc_id(next_doc_id);

	et_canvas_collection_delete_canvases_from_doc_id(doc_id);

	et_doc_manager_delete_doc_from_id(doc_id);

	et_debug("%d %d", doc_id, next_doc_id);
	et_debug("Closed :%d", doc_id);
}

static bool _confirm_dialog_run(
		GtkWidget *window,
		const char *dialog_title,
		const char *message,
		const char *accept_button_title)
{
	et_assert(dialog_title);
	et_assert(message);
	et_assert(accept_button_title);

	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *dialog = gtk_dialog_new_with_buttons (
			dialog_title,
			GTK_WINDOW(window),
			flags,
			_("_Cancel"),
			GTK_RESPONSE_CANCEL,
			accept_button_title,
			GTK_RESPONSE_ACCEPT,
			NULL);
	GtkWidget *message_label = gtk_label_new(message);
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), message_label);
	gtk_widget_show_all(message_label);
	gint res = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	return (res == GTK_RESPONSE_ACCEPT);
}

