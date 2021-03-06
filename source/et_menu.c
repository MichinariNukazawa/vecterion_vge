/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#include "et_menu.h"

#include <glib/gi18n.h>
#include <string.h>

#include "et_error.h"
#include "et_define.h"
#include "et_canvas.h"
#include "et_canvas_collection.h"
#include "et_doc.h"
#include "et_doc_manager.h"
#include "et_etaion.h"
#include "et_tool_info.h"
#include "et_tool_panel.h"
#include "pv_io.h"
#include "pv_renderer.h"
#include "et_color_panel.h"
#include "et_stroke_panel.h"
#include "et_position_panel.h"
#include "pv_file_format.h"
#include "et_doc_relation.h"
#include "et_clipboard_manager.h"
#include "et_document_preference_dialog.h"
#include "version.h"

typedef struct{
	GtkWidget *menuitem_snap_for_grid;
}EtMenu;

EtMenu self_;
EtMenu *self = &self_;

static EtWindow *et_window_ = NULL;

#define _show_error_dialog(fmt, ...) \
	do{ \
		et_error(""fmt"", ## __VA_ARGS__); \
		char *message = g_strdup_printf(""fmt"", ## __VA_ARGS__ ); \
		show_error_dialog_impl_(message); \
		g_free(message); \
	}while(0)
static void show_error_dialog_impl_(const char *message)
{
	et_assert(message);

	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *dialog = gtk_message_dialog_new (
			et_window_->window,
			flags,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			"%s",
			message);
	et_assert(dialog);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	return;
}

EtDocId open_doc_new_from_file_(const char* filepath, const PvImageFileReadOption *imageFileReadOption)
{
	et_debug("filepath:'%s'", (NULL == filepath)? "NULL":filepath);

	if(NULL == filepath){
		et_error("");
		return -1;
	}

	const PvFileFormat *format = get_file_format_from_filepath(filepath);
	if(!format){
		et_error("");
		return -1;
	}

	PvVg *vg_src = NULL;
	EtDocId doc_id = -1;
	if( PvFormatKind_SVG == format->kind){
		vg_src = pv_io_new_from_file(filepath, imageFileReadOption);
		if(NULL == vg_src){
			et_error("");
			return -1;
		}
	}else{
		vg_src = pv_vg_new();
		if(NULL == vg_src){
			et_error("");
			return -1;
		}
		PvElement *element_parent = pv_vg_get_layer_top(vg_src);
		if(NULL == element_parent){
			et_error("");
			goto finally;
		}
		PvElement *element_basic_shape = pv_element_basic_shape_new_from_filepath(filepath);
		if(NULL == element_basic_shape){
			et_error("");
			goto finally;
		}
		if(! pv_element_append_child(element_parent, NULL, element_basic_shape)){
			et_error("");
			goto finally;
		}

		PvElementBasicShapeData *element_data = element_basic_shape->data;
		PvRasterData *data = element_data->data;
		vg_src->rect.w = gdk_pixbuf_get_width(data->pixbuf);
		vg_src->rect.h = gdk_pixbuf_get_height(data->pixbuf);
	}

	doc_id = open_doc_new(vg_src);
	if(0 > doc_id){
		et_error("");
		goto finally;
	}

	if(!et_doc_set_saved_filepath(doc_id, filepath)){
		et_bug("");
	}

finally:
	pv_vg_free(vg_src);
	et_doc_signal_update_from_id(doc_id);

	return doc_id;
}

static gboolean cb_menu_file_new_(gpointer data)
{
	et_debug("");

	// 作成ドキュメント情報
	PvVg *vg = pv_vg_new();
	if(NULL == vg){
		et_error("");
		return false;
	}
	vg->rect.x = 0;
	vg->rect.y = 0;
	vg->rect.w = 1200;
	vg->rect.h = 800;

	GtkWidget *spin_w = GTK_WIDGET(gtk_builder_get_object(
				et_window_->document_new_dialog_builder, "spinbutton_w"));
	et_assert(spin_w);
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spin_w), PVVG_PX_SIZE_MIN, PVVG_PX_SIZE_MAX);
	GtkWidget *spin_h = GTK_WIDGET(gtk_builder_get_object(
				et_window_->document_new_dialog_builder, "spinbutton_h"));
	et_assert(spin_h);
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spin_h), PVVG_PX_SIZE_MIN, PVVG_PX_SIZE_MAX);
	GtkWidget *dialog = GTK_WIDGET(gtk_builder_get_object(
				et_window_->document_new_dialog_builder, "document_new_dialog"));
	et_assert(dialog);

	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	switch (result)
	{
		case GTK_RESPONSE_ACCEPT:
			{
				vg->rect.w = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_w));
				vg->rect.h = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_h));
				et_debug("size:%f,%f,%f,%f",
						vg->rect.x, vg->rect.y, vg->rect.w, vg->rect.h);
				open_doc_new(vg);
			}
			break;
		default:
			et_debug("Cancel");
			break;
	}
	gtk_widget_hide (dialog);

	pv_vg_free(vg);

	return false;
}

static bool output_svg_from_doc_id_(const char *filepath, EtDocId doc_id)
{
	if(NULL == filepath){
		et_bug("");
		goto error;
	}


	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_debug("%d", doc_id);
		goto error;
	}

	PvVg *vg = et_doc_get_vg_ref(doc);
	if(NULL == vg){
		et_debug("%d", doc_id);
		goto error;
	}

	if(!pv_io_write_file_svg_from_vg(vg, filepath)){
		et_debug("%d", doc_id);
		goto error;
	}

	return true;

error:
	return false;
}

bool output_file_from_doc_id_(const char *filepath, EtDocId doc_id)
{
	if(NULL == filepath){
		et_error("");
		return false;
	}

	const PvFileFormat *format = get_file_format_from_filepath(filepath);
	if(!format){
		et_error("");
		return false;
	}

	if(0 == strcmp("svg", format->gdk_file_type)){
		if(!output_svg_from_doc_id_(filepath, doc_id)){
			et_error("");
			return false;
		}
	}else{
		PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
		if(!vg){
			et_error("");
			return false;
		}
		PvRenderContext render_context = PvRenderContext_Default;
		if(format->has_alpha){
			render_context.background_kind = PvBackgroundKind_Transparent;
		}else{
			render_context.background_kind = PvBackgroundKind_White;
		}
		GdkPixbuf *pixbuf = pv_renderer_pixbuf_from_vg(
				vg,
				render_context,
				NULL,
				NULL,
				NULL);
		if(!pixbuf){
			et_error("");
			return false;
		}

		bool ret = true;
		GError *error = NULL;
		switch(format->kind){
			case PvFormatKind_JPEG:
				ret = gdk_pixbuf_save(
						pixbuf,
						filepath,
						format->gdk_file_type,
						&error,
						"quality", "100", NULL);
				break;
			case PvFormatKind_PNG:
				ret = gdk_pixbuf_save(
						pixbuf,
						filepath,
						format->gdk_file_type,
						&error,
						"compression", "0", NULL);
				break;
			default:
				ret = gdk_pixbuf_save(
						pixbuf,
						filepath,
						format->gdk_file_type,
						&error,
						NULL);
				break;
		}

		if(!ret){
			et_error("error:'%s','%s'", filepath, error->message);
			return false;
		}
	}

	return true;
}

static char *get_dirpath_from_filepath_(const char *filepath)
{
	if(NULL == filepath){
		return NULL;
	}

	char *dirpath = g_strdup(filepath);
	et_assert(dirpath);

	char *sep = strrchr(dirpath, G_DIR_SEPARATOR);
	if(NULL == sep){
		g_free(dirpath);
		return NULL;
	}

	*sep = '\0';

	return dirpath;
}

/*! @return Acceps:new string of filepath(need g_free()). Cancel:NULL */
static char *save_dialog_run_(const char *dialog_title, const char *accept_button_title, const char *default_filepath)
{
	et_assert(default_filepath);
	et_debug("'%s'", default_filepath);

	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
	GtkWidget *dialog = gtk_file_chooser_dialog_new (dialog_title,
			et_window_->window,
			action,
			_("_Cancel"),
			GTK_RESPONSE_CANCEL,
			accept_button_title,
			GTK_RESPONSE_ACCEPT,
			NULL);
	et_assert(dialog);
	GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
	et_assert(chooser);

	gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

	char *filename = strrchr(default_filepath, G_DIR_SEPARATOR);
	if(NULL == filename){
		gtk_file_chooser_set_current_name (chooser, default_filepath);
	}else{
		et_debug("'%s'", filename);
		char *dirpath = get_dirpath_from_filepath_(default_filepath);
		if(NULL != dirpath){
			et_debug("'%s'", dirpath);
			gtk_file_chooser_set_current_folder (chooser, dirpath);
			g_free(dirpath);
		}
		filename++;
		gtk_file_chooser_set_current_name (chooser, filename);
	}

	char *filepath = NULL;
	gint res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		filepath = gtk_file_chooser_get_filename (chooser);
	}

	gtk_widget_destroy (dialog);

	return filepath;
}

static gboolean cb_menu_file_save_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Save:nothing document.");
		et_bug("%d", doc_id);
		return false;
	}

	et_doc_save_from_id(doc_id);

	char *filepath = NULL;
	if(!et_doc_get_saved_filepath(&filepath, doc_id)){
		_show_error_dialog("Save:internal error.");
		goto finally;
	}

	if(NULL != filepath){
		// ** change to default extension
		const PvFileFormat *format = get_file_format_from_filepath(filepath);
		if(NULL == format || false == format->is_native){
			char *next_filepath = NULL;
			next_filepath = pv_file_format_change_new_extension_from_filepath(filepath, "svg");
			g_free(filepath);
			filepath = next_filepath;

			next_filepath = save_dialog_run_("Save File", _("_Save"), filepath);
			g_free(filepath);
			filepath = next_filepath;
		}
	}

	if(NULL == filepath){
		// ** user select filepath
		filepath = save_dialog_run_("Save File", _("_Save"), _("untitled_document.svg"));
	}

	if(NULL == filepath){
		et_debug("Cancel");
		goto finally;
	}

	// ** check extension
	const PvFileFormat *format = get_file_format_from_filepath(filepath);
	if(NULL == format || false == format->is_native){
		_show_error_dialog("Save:can not native format(please use Export).'%s'", filepath);
		goto finally;
	}

	if(!et_doc_set_saved_filepath(doc_id, filepath)){
		_show_error_dialog("Save:'%s'", filepath);
		goto finally;
	}
	if(!output_svg_from_doc_id_(filepath, doc_id)){
		_show_error_dialog("Save:'%s'", filepath);
		goto finally;
	}

	et_debug("Save:'%s'", filepath);

finally:
	if(NULL != filepath){
		g_free (filepath);
	}

	return false;
}

static gboolean cb_menu_file_save_as_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Save:nothing document.");
		et_bug("%d", doc_id);
		return false;
	}

	et_doc_save_from_id(doc_id);

	char *filepath = NULL;
	char *src_filepath = NULL;
	if(!et_doc_get_saved_filepath(&src_filepath, doc_id)){
		_show_error_dialog("Save:internal error.");
		goto finally;
	}

	if(NULL != src_filepath){
		// ** change to default extension
		const PvFileFormat *format = get_file_format_from_filepath(src_filepath);
		if(NULL == format || false == format->is_native){
			char *next_filepath = pv_file_format_change_new_extension_from_filepath(src_filepath, "svg");
			g_free(src_filepath);
			src_filepath = next_filepath;
		}
	}

	char *tmp_filepath = ((src_filepath)? src_filepath : _("untitled_document.svg"));
	// ** user select filepath
	filepath = save_dialog_run_("Save File", _("_Save"), tmp_filepath);

	if(NULL == filepath){
		et_debug("Cancel");
		goto finally;
	}

	// ** check extension
	const PvFileFormat *format = get_file_format_from_filepath(filepath);
	if(NULL == format || false == format->is_native){
		_show_error_dialog("Save:can not native format(please use Export).'%s'", filepath);
		goto finally;
	}

	if(!output_svg_from_doc_id_(filepath, doc_id)){
		_show_error_dialog("Save:'%s'", filepath);
		goto finally;
	}
	EtDocId dst_doc_id = open_doc_new_from_file_(filepath, &PvImageFileReadOption_Default);
	if(dst_doc_id < 0){
		_show_error_dialog("Save:'%s'", filepath);
		goto finally;
	}

	et_debug("Save:'%s'", filepath);

finally:
	if(NULL != src_filepath){
		g_free (src_filepath);
	}
	if(NULL != filepath){
		g_free (filepath);
	}

	return false;
}

static gboolean cb_menu_file_export_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Export:nothing document.");
		et_bug("%d", doc_id);
		return false;
	}

	char *filepath = NULL;

	char *prev_filepath = NULL;
	if(!et_doc_get_saved_filepath(&prev_filepath, doc_id)){
		_show_error_dialog("Export:internal error.");
		goto finally;
	}

	const PvFileFormat *format = get_file_format_from_filepath(filepath);
	if(NULL == format || true == format->is_native){
		char *next_filepath = pv_file_format_change_new_extension_from_filepath(prev_filepath, "png");
		g_free(prev_filepath);
		prev_filepath = next_filepath;
	}

	filepath = save_dialog_run_("Export File", _("_Export"), prev_filepath);
	if(NULL == filepath){
		// cancel
		et_debug("Cancel:%s", prev_filepath);
		goto finally;
	}

	if(!output_file_from_doc_id_(filepath, doc_id)){
		_show_error_dialog("Export:'%s'", filepath);
		goto finally;
	}

	et_debug("Export:'%s'", filepath);

finally:
	if(NULL != prev_filepath){
		g_free(prev_filepath);
	}
	if(NULL != filepath){
		g_free(filepath);
	}

	return false;
}

static gboolean cb_menu_file_close_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	close_doc_from_id(doc_id, et_window_->status_bar);

	return false;
}

static gboolean cb_menu_file_quit_(gpointer data)
{
	gui_quit_();
	return FALSE;
}

static gboolean cb_menu_edit_undo_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	if(!et_doc_undo_from_id(doc_id)){
		et_error("");
		return false;
	}

	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean cb_menu_edit_redo_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	if(!et_doc_redo_from_id(doc_id)){
		et_error("");
		return false;
	}

	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean cb_menu_edit_cut_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	if(!et_clipboard_cut_from_doc_id(doc_id)){
		et_error("");
		return false;
	}

	et_doc_save_from_id(doc_id);
	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean cb_menu_edit_copy_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	if(!et_clipboard_copy_from_doc_id(doc_id)){
		et_error("");
		return false;
	}

	et_doc_save_from_id(doc_id);
	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean cb_menu_edit_paste_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	if(!et_clipboard_paste_from_doc_id(doc_id)){
		et_error("");
		return false;
	}

	et_doc_save_from_id(doc_id);
	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean cb_menu_edit_delete_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	if(!et_etaion_remove_delete_by_focusing(doc_id)){
		et_error("");
		return false;
	}

	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean cb_menu_document_preference_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("DocumentPreferenceDialog:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	pv_document_preference_dialog_run(
			et_window_->document_preference_dialog_builder);

	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean cb_menu_layer_new_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	et_etaion_append_new_layer(doc_id);

	return false;
}

static gboolean cb_menu_layer_copy_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	et_etaion_copy_layer(doc_id);

	return false;
}

static gboolean cb_menu_layer_new_child_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	et_etaion_append_new_layer_child(doc_id);

	return false;
}

static gboolean cb_menu_layer_delete_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d", doc_id);
		return false;
	}

	et_etaion_remove_delete_layer(doc_id);

	return false;
}

static void reordering_element_(EtDocId doc_id, int move, bool is_end, bool is_layer)
{
	et_assertf((1 == move || -1 == move), "%d", move);

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	size_t num_focus = pv_general_get_parray_num((void **)focus->elements);
	if(1 != num_focus){
		_show_error_dialog("Reorder:element not ones.");
		return;
	}

	PvElement *parent_element = NULL;
	PvElement *target_element = NULL;
	if(is_layer){
		target_element = pv_element_get_first_parent_layer_or_root(focus->elements[0]);
		et_assert(target_element);
		parent_element = target_element->parent;
		et_assert(parent_element);
	}else{
		target_element = focus->elements[0];
		et_assert(target_element);
		parent_element = target_element->parent;
		et_assert(parent_element);

		if(!pv_element_kind_is_object(target_element->kind)){
			_show_error_dialog("Reorder:element not ones.");
			et_debug("%d", focus->elements[0]->kind);
			return;
		}
	}

	size_t num_childs = pv_general_get_parray_num((void **)parent_element->childs);
	for(int i = 0; i < (int)num_childs; i++){
		if(target_element == parent_element->childs[i]){
			int index = i + (-1 * move);
			if(is_end){
				if(1 == move){
					index = 0;
				}else{
					index = (int)num_childs - 1;
				}
			}
			if(index < 0 || (int)num_childs <= index){
				// NOP
				return;
			}else{
				bool ret;
				PvElement *element = parent_element->childs[i];
				et_assertf(element, "%d", i);
				ret = pv_element_remove(element);
				et_assertf(ret, "%d", i);
				ret = pv_element_append_nth(parent_element, index, element);
				et_assertf(ret, "%d", i);

				et_doc_save_from_id(doc_id);
				et_doc_signal_update_from_id(doc_id);

				return;
			}
		}
	}

	et_abortf("");
}

static gboolean cb_menu_layer_raise_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return false;
	}

	reordering_element_(doc_id, -1, false, true);

	return false;
}

static gboolean cb_menu_layer_lower_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return false;
	}

	reordering_element_(doc_id, 1, false, true);

	return false;
}

static gboolean cb_menu_layer_raise_to_top_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return false;
	}

	reordering_element_(doc_id, -1, true, true);

	return false;
}

static gboolean cb_menu_layer_lower_to_end_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return false;
	}

	reordering_element_(doc_id, 1, true, true);

	return false;
}

static gboolean cb_menu_element_raise_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return false;
	}

	reordering_element_(doc_id, -1, false, false);

	return false;
}

static gboolean cb_menu_element_lower_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return false;
	}

	reordering_element_(doc_id, 1, false, false);

	return false;
}

static gboolean cb_menu_element_raise_to_top_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return false;
	}

	reordering_element_(doc_id, -1, true, false);

	return false;
}

static gboolean cb_menu_element_lower_to_end_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return false;
	}

	reordering_element_(doc_id, 1, true, false);

	return false;
}

static bool cb_menu_element_grouping_inline_(EtDocId doc_id)
{
	if(doc_id < 0){
		return false;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	size_t num = pv_general_get_parray_num((void **)focus->elements);
	et_assert(0 != num);

	for(int i = 0; i < (int)num; i++){
		switch(focus->elements[i]->kind){
			case PvElementKind_Root:
			case PvElementKind_Layer:
				_show_error_dialog("Grouping:focus included layer.");
				return true;
				break;
			default:
				break;
		}
	}

	PvElement *element_group = pv_element_new(PvElementKind_Group);
	et_assert(element_group);

	PvElement *first_element = pv_focus_get_first_element(focus);
	bool ret = pv_element_append_child(
			first_element->parent,
			first_element,
			element_group);
	if(!ret){
		et_bug("%zu", num);
		pv_element_free(element_group);
		goto finally;
	}

	for(int i = 0; i < (int)num; i++){
		pv_element_remove(focus->elements[i]);
		pv_element_append_child(
				element_group,
				NULL,
				focus->elements[i]);
	}

	pv_focus_add_element(focus, element_group);

finally:
	return true;
}


static gboolean cb_menu_element_grouping_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return FALSE;
	}

	if(!cb_menu_element_grouping_inline_(doc_id)){
		et_warning("");
		return FALSE;
	}

	et_doc_save_from_id(doc_id);
	et_doc_signal_update_from_id(doc_id);

	return FALSE;
}

static PvElement *get_first_parent_group_(PvElement *element)
{
	switch(element->kind){
		case PvElementKind_Group:
			return element;
			break;
		case PvElementKind_Root:
		case PvElementKind_Layer:
			return NULL;
			break;
		default:
			return get_first_parent_group_(element->parent);
	}
}

static gboolean cb_menu_element_ungrouping_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return false;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	size_t num_focus = pv_general_get_parray_num((void **)focus->elements);
	et_assert(0 != num_focus);

	PvElement *element_group = NULL;
	if(1 != num_focus){
		element_group = pv_focus_get_first_element(focus);
		if(PvElementKind_Group != element_group->kind){
			_show_error_dialog("Ungrouping:element not ones or group.");
			return false;
		}
	}else{
		element_group = get_first_parent_group_(pv_focus_get_first_element(focus));
	}
	if(NULL == element_group){
		_show_error_dialog("Ungrouping:element not group.");
		return false;
	}

	pv_focus_clear_to_first_layer(focus);

	size_t num = pv_general_get_parray_num((void **)element_group->childs);
	PvElement **elements = pv_element_copy_elements(element_group->childs);
	et_assert(elements);
	for(int i = 0; i <(int)num; i++){
		PvElement *element = elements[i];
		pv_element_remove(element);
		pv_element_append_child(
				element_group->parent,
				element_group,
				element);
		pv_focus_add_element(focus, element);
	}
	free(elements);

	pv_element_remove_free_recursive(element_group);

	et_doc_save_from_id(doc_id);
	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean cb_menu_element_mask_grouping_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// _show_error_dialog("Raise:nothing document.");
		// et_bug("%d", doc_id);
		return FALSE;
	}

	if(!cb_menu_element_grouping_inline_(doc_id)){
		return FALSE;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	size_t num_focus = pv_general_get_parray_num((void **)focus->elements);
	et_assert(0 != num_focus);

	PvElement *element = pv_focus_get_first_element(focus);
	et_assertf(PvElementKind_Group == element->kind, "%d", element->kind);

	PvElementGroupData *group_data = element->data;
	group_data->kind = PvElementGroupKind_MaskCurveSimple;
	group_data->cairo_fill_rule = CAIRO_FILL_RULE_EVEN_ODD;

	et_doc_save_from_id(doc_id);
	et_doc_signal_update_from_id(doc_id);

	return FALSE;
}

bool gui_quit_()
{
	EtDocId doc_id = -1;
	while(-1 != (doc_id = et_etaion_get_current_doc_id())){
		if(!close_doc_from_id(doc_id, et_window_->status_bar)){
			// Cancel
			et_debug("Quit: canceled.");
			return false;
		}
	}

	et_debug("Quit");
	gtk_main_quit();

	return true;
}

static gboolean cb_menu_file_open_(gpointer data)
{
	et_debug("");

	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open File",
			NULL,
			action,
			_("_Cancel"),
			GTK_RESPONSE_CANCEL,
			_("_Open"),
			GTK_RESPONSE_ACCEPT,
			NULL);
	et_assert(dialog);

	gint res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		filename = gtk_file_chooser_get_filename (chooser);
		EtDocId doc_id = open_doc_new_from_file_(filename, &PvImageFileReadOption_Default);
		if(doc_id < 0){
			_show_error_dialog("Open:open error.:'%s'", filename);
		}
		g_free (filename);
	}

	gtk_widget_destroy (dialog);



	return false;
}

static bool pv_element_append_near_first_parrent_layer_(PvFocus *focus, PvElement *element)
{
	PvElement *first_parent = pv_focus_get_first_layer(focus);

	PvElement *sister_layer = NULL;
	PvElement *parent_layer = NULL;
	if(element->kind == PvElementKind_Layer
			&& first_parent->kind != PvElementKind_Root){
		//! PvElementKind_Layer is append sister level on current layer
		sister_layer = first_parent;
		parent_layer = sister_layer->parent;
	}else{
		//! PvElementKind_Layer is append child level on current layer
		sister_layer = NULL;
		parent_layer = first_parent;
	}

	return pv_element_append_child(parent_layer, sister_layer, element);
}

static PvElement *new_element_from_filepath_(const char *filepath)
{
	PvElement *element = NULL;
	const PvFileFormat *format = get_file_format_from_filepath(filepath);
	if(!format){
		et_warning("%s", filepath);
		return NULL;
	}

	if(PvFormatKind_SVG == format->kind){
		element = pv_io_new_element_from_filepath(filepath);
	}else{
		element = pv_element_basic_shape_new_from_filepath(filepath);
	}

	if(NULL == element){
		et_warning("%s", filepath);
	}

	return element;
}

static gboolean cb_menu_file_import_(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Import:nothing document.");
		et_bug("%d", doc_id);
		return false;
	}

	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Import File",
			NULL,
			action,
			_("_Cancel"),
			GTK_RESPONSE_CANCEL,
			_("_Import"),
			GTK_RESPONSE_ACCEPT,
			NULL);
	et_assert(dialog);

	char *filename = NULL;
	gint res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		filename = gtk_file_chooser_get_filename (chooser);

		PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
		if(NULL == focus){
			_show_error_dialog("Import:internal error.");
			et_debug("%d", doc_id);
			goto finally_1;
		}

		PvElement *import_element = new_element_from_filepath_(filename);
		if(NULL == import_element){
			_show_error_dialog("Import:open error.:%s", filename);
			et_debug("%d", doc_id);
			goto finally_1;
		}

		if(!pv_element_append_near_first_parrent_layer_(focus, import_element)){
			pv_element_remove_free_recursive(import_element);

			_show_error_dialog("Import:open error.:%s", filename);
			et_debug("%d", doc_id);
			goto finally_1;
		}

		if(!pv_focus_clear_set_element(focus, import_element)){
			_show_error_dialog("Import:open error.:%s", filename);
			et_debug("%d", doc_id);
			goto finally_1;
		}
		if(!pv_focus_clear_to_first_layer(focus)){
			_show_error_dialog("Import:open error.:%s", filename);
			et_debug("%d", doc_id);
			goto finally_1;
		}

		g_free (filename);
	}

	gtk_widget_destroy (dialog);

	et_doc_save_from_id(doc_id);
	et_doc_signal_update_from_id(doc_id);

	et_debug("Import");

	return false;

finally_1:
	if(NULL != filename){
		g_free (filename);
	}
	gtk_widget_destroy (dialog);
	return false;

}

static void cb_menu_view_zoom_in_(GtkCheckMenuItem *menuitem, gpointer user_data)
{
	EtCanvas *current_canvas = et_canvas_collection_get_current_canvas();
	if(NULL == current_canvas){
		_show_error_dialog("Zoom:nothing document.");
		return;
	}
	et_canvas_change_scale_of_unit(current_canvas, 1);
}

static void cb_menu_view_zoom_out_(GtkCheckMenuItem *menuitem, gpointer user_data)
{
	EtCanvas *current_canvas = et_canvas_collection_get_current_canvas();
	if(NULL == current_canvas){
		_show_error_dialog("Zoom:nothing document.");
		return;
	}
	et_canvas_change_scale_of_unit(current_canvas, -1);
}

static void cb_menu_view_extent_(GtkCheckMenuItem *menuitem, gpointer user_data)
{
	if(!et_etaion_set_is_extent_view(gtk_check_menu_item_get_active(menuitem))){
		et_error("");
	}
}

static void cb_menu_view_transparent_grid_(GtkCheckMenuItem *menuitem, gpointer user_data)
{
	et_etaion_set_is_transparent_grid(gtk_check_menu_item_get_active(menuitem));
}

static bool cb_menu_document_resize_ (GtkMenuItem *menuitem, gpointer user_data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Resize:nothing document.");
		et_bug("%d", doc_id);
		return false;
	}

	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	if(NULL == vg){
		_show_error_dialog("Resize:internal error.");
		et_debug("%d", doc_id);
		return false;
	}

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *dialog = gtk_dialog_new_with_buttons ("Resize Document",
			NULL,
			flags,
			"_OK",
			GTK_RESPONSE_ACCEPT,
			"_Cancel",
			GTK_RESPONSE_REJECT,
			NULL);
	et_assert(dialog);

	GtkWidget *hbox_w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	GtkWidget *hbox_h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	GtkWidget *spin_w = gtk_spin_button_new_with_range(0, PVVG_PX_SIZE_MAX, PVVG_PX_SIZE_MIN);
	GtkWidget *spin_h = gtk_spin_button_new_with_range(0, PVVG_PX_SIZE_MAX, PVVG_PX_SIZE_MIN);
	GtkWidget *label_w = gtk_label_new_with_mnemonic("width ");
	GtkWidget *label_h = gtk_label_new_with_mnemonic("height");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_w), vg->rect.w);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_h), vg->rect.h);
	GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_pack_start(GTK_BOX(hbox_w), label_w, true, true, 0);
	gtk_box_pack_start(GTK_BOX(hbox_h), label_h, true, true, 1);
	gtk_box_pack_end(GTK_BOX(hbox_w), spin_w, true, true, 0);
	gtk_box_pack_end(GTK_BOX(hbox_h), spin_h, true, true, 1);
	gtk_box_pack_start(GTK_BOX(content), hbox_w, true, true, 0);
	gtk_box_pack_start(GTK_BOX(content), hbox_h, true, true, 1);
	gtk_widget_show_all(dialog);

	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	switch (result)
	{
		case GTK_RESPONSE_ACCEPT:
			{
				vg->rect.w = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_w));
				vg->rect.h = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_h));
			}
			break;
		default:
			et_debug("Cancel");
			break;
	}
	gtk_widget_destroy (dialog);

	et_doc_save_from_id(doc_id);
	et_doc_signal_update_from_id(doc_id);

	et_debug("Resize");

	return false;
}

static bool cb_menu_tool_change_(GtkMenuItem *menuitem, gpointer user_data)
{
	et_assert(user_data);
	EtToolInfo *info = user_data;

	bool ret = slot_et_etaion_change_tool(info->tool_id, NULL);
	et_assertf(ret, "%d", info->tool_id);

	return false;
}

static bool cb_menu_tool_snap_for_grid_(GtkCheckMenuItem *menuitem, gpointer user_data)
{
	et_debug("");

	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Resize:nothing document.");
		et_bug("%d", doc_id);
		return false;
	}

	PvDocumentPreference document_preference = et_doc_get_document_preference_from_id(doc_id);
	document_preference.snap_context.is_snap_for_grid = gtk_check_menu_item_get_active(menuitem);
	et_doc_set_document_preference_from_id(doc_id, document_preference);

	et_doc_signal_update_from_id(doc_id);

	return false;
}

static bool pv_element_is_exist_from_elements_(const PvElement *element, PvElement **elements)
{
	size_t num = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < (int)num; i++){
		if(element == elements[i]){
			return true;
		}
	}

	return false;
}

typedef struct{
	PvFocus *focus;
	PvElement **elements_ignore;
}EtSelectAllFuncRecurseDataPack;

static bool cb_menu_select_all_func_recurse_prev_(PvElement *element, gpointer data, int level)
{
	EtSelectAllFuncRecurseDataPack *func_safr_data_pack = data;
	PvFocus *focus = func_safr_data_pack->focus;

	if(PvElementKind_Layer == element->kind){
		return true;
	}

	if(PvElementKind_Root == element->kind){
		return true;
	}

	if(pv_element_is_exist_from_elements_(element, func_safr_data_pack->elements_ignore)){
		return true;
	}

	if(pv_element_get_in_is_invisible(element)){
		return true;
	}

	if(pv_element_get_in_is_locked(element)){
		return true;
	}

	if(!pv_focus_add_element(focus, element)){
		return false;
	}

	return true;
}

static void cb_menu_select_all_ (GtkMenuItem *menuitem, gpointer user_data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Select All:nothing document.");
		et_bug("%d", doc_id);
		return;
	}

	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	if(NULL == vg){
		_show_error_dialog("Select All:internal error.");
		et_debug("%d", doc_id);
		return;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		_show_error_dialog("Select All:internal error.");
		et_bug("");
		return;
	}
	EtSelectAllFuncRecurseDataPack func_safr_data_pack = {
		.focus = focus,
		.elements_ignore = NULL,
	};
	PvElementRecursiveError error;
	if(!pv_element_recursive_desc_before(
				vg->element_root,
				cb_menu_select_all_func_recurse_prev_,
				&func_safr_data_pack,
				&error))
	{
		_show_error_dialog("Select All:internal error.");
		et_error("level:%d", error.level);
		return;
	}

	et_doc_signal_update_from_id(doc_id);
}

static void cb_menu_select_none_ (GtkMenuItem *menuitem, gpointer user_data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Select:nothing document.");
		et_bug("%d", doc_id);
		return;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		_show_error_dialog("Select:internal error.");
		et_bug("");
		return;
	}

	if(!pv_focus_clear_to_first_layer(focus)){
		_show_error_dialog("Select:internal error.");
		et_error("");
		return;
	}

	et_doc_signal_update_from_id(doc_id);
}

static void cb_menu_select_invert_ (GtkMenuItem *menuitem, gpointer user_data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Select Invert:nothing document.");
		et_bug("%d", doc_id);
		return;
	}

	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	if(NULL == vg){
		_show_error_dialog("Select Invert:internal error.");
		et_debug("%d", doc_id);
		return;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		_show_error_dialog("Select Invert:internal error.");
		et_bug("");
		return;
	}

	size_t num = pv_general_get_parray_num((void **)focus->elements);
	PvElement **elements_prefocus = malloc(sizeof(PvElement *) * (num + 1));
	if(NULL == elements_prefocus){
		_show_error_dialog("Select Invert:internal error.");
		et_bug("");
		return;
	}
	memcpy(elements_prefocus, focus->elements, sizeof(PvElement *) * (num + 1));

	if(!pv_focus_clear_to_first_layer(focus)){
		_show_error_dialog("Select Invert:internal error.");
		et_error("");
		goto finally;
	}

	EtSelectAllFuncRecurseDataPack func_safr_data_pack = {
		.focus = focus,
		.elements_ignore = elements_prefocus,
	};
	PvElementRecursiveError error;
	if(!pv_element_recursive_desc_before(
				vg->element_root,
				cb_menu_select_all_func_recurse_prev_,
				&func_safr_data_pack,
				&error))
	{
		_show_error_dialog("Select Invert:internal error.");
		et_error("level:%d", error.level);
		goto finally;
	}

finally:
	free(elements_prefocus);

	et_doc_signal_update_from_id(doc_id);
}

#include <libxml/xmlversion.h>
static void cb_menu_help_about_ (GtkMenuItem *menuitem, gpointer user_data)
{
	gchar *gtk_version = g_strdup_printf("libgtk:%d.%2d.%2d %s",
			gtk_get_major_version(),
			gtk_get_minor_version(),
			gtk_get_micro_version(),
			"LGPL"
			);
	gchar *libxml_version = g_strdup_printf("libxml:%s %s",
			LIBXML_DOTTED_VERSION,
			"MIT License"
			);
	gchar *versions = g_strdup_printf("%s\n%s",
			gtk_version,
			libxml_version
			);

	GtkWindow *parent_window = NULL;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *dialog = gtk_message_dialog_new (parent_window,
			flags,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_CLOSE,
			"`%s`\n%s",
			VECTERION_FULLNAME,
			et_window_->vecterion_build);
	et_assert(dialog);

	GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	et_assert(content_area);
	GtkWidget *frame_ = gtk_frame_new (NULL);
	gtk_frame_set_label(GTK_FRAME (frame_), "Library");
	gtk_container_add(GTK_CONTAINER(content_area), frame_);
	GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_hexpand(GTK_WIDGET(scroll), TRUE);
	gtk_widget_set_vexpand(GTK_WIDGET(scroll), TRUE);
	gtk_widget_set_size_request(scroll, 300, 100);
	gtk_container_add(GTK_CONTAINER(frame_), scroll);
	GtkWidget *text = gtk_text_view_new();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text));
	gtk_text_buffer_set_text (buffer, versions, -1);
	gtk_text_view_set_editable (GTK_TEXT_VIEW(text), false);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(text), false);
	gtk_container_add(GTK_CONTAINER(scroll), text);

	static GdkPixbuf *daisy_bell_header = NULL;
	if(NULL == daisy_bell_header){
		char *filepath = g_strdup_printf(
				"%s/%s",
				et_etaion_get_application_base_dir(),
				"resource/vecterion/daisy_bell_header_r2.jpg"
				);
		GError *error = NULL;
		et_assert(filepath);

		GdkPixbuf *t_daisy_bell_header = gdk_pixbuf_new_from_file(filepath, &error);
		if(NULL == t_daisy_bell_header){
			et_error("'%s'", error->message);
			g_clear_error(&error);
		}
		free(filepath);

		daisy_bell_header = gdk_pixbuf_scale_simple(
				t_daisy_bell_header,
				960 * 0.4, 500 * 0.4,
				GDK_INTERP_HYPER);
		et_assert(daisy_bell_header);
	}
	if(NULL != daisy_bell_header){
		GtkWidget *image = gtk_image_new_from_pixbuf(daisy_bell_header);
		et_assert(image);
		gtk_container_add(GTK_CONTAINER(content_area), image);
	}

	gtk_widget_show_all(dialog);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	g_free(versions);
	g_free(libxml_version);
	g_free(gtk_version);
}

static GtkWidget *pv_get_menuitem_new_tree_of_select_(GtkAccelGroup *accel_group){
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	menuitem_root = gtk_menu_item_new_with_label ("_Select");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "/_Select/_All (Ctrl+A)"
	menuitem = gtk_menu_item_new_with_label ("_All");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_select_all_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	// ** Accel to "/_Select/_None (Shift+Ctrl+A)"
	menuitem = gtk_menu_item_new_with_label ("_None");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_select_none_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_a, (GDK_SHIFT_MASK | GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);
	// ** Accel to "/_Select/_Invert (Ctrl+I)"
	menuitem = gtk_menu_item_new_with_label ("_Invert");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_select_invert_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

static GtkWidget *pv_get_menuitem_new_tree_of_file_(GtkAccelGroup *accel_group){
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	menuitem_root = gtk_menu_item_new_with_label ("_File");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "/_File/_New (Ctrl+N)"
	menuitem = gtk_menu_item_new_with_label ("_New");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_file_new_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// ** "/_File/_Open (Ctrl+O)"
	menuitem = gtk_menu_item_new_with_label ("_Open");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_file_open_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// ** "/_File/_Import"
	menuitem = gtk_menu_item_new_with_label ("_Import");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_file_import_), NULL);

	// ** "/_File/_Save (Ctrl+S)"
	menuitem = gtk_menu_item_new_with_label ("_Save");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_file_save_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// ** "/_File/Save _As (Ctrl+Shift+S)"
	menuitem = gtk_menu_item_new_with_label ("Save _As");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_file_save_as_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_s, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** "/_File/_Export (Ctrl+Shift+E)"
	menuitem = gtk_menu_item_new_with_label ("_Export");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_file_export_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_e, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** "/_File/_Close (Ctrl+W)"
	menuitem = gtk_menu_item_new_with_label ("_Close");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_file_close_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_w, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	// ** "/_File/_Quit (Ctrl+Q)"
	menuitem = gtk_menu_item_new_with_label ("_Quit");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_file_quit_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_q, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

static GtkWidget *pv_get_menuitem_new_tree_of_edit_(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;
	GtkWidget *separator;

	menuitem_root = gtk_menu_item_new_with_label ("_Edit");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "/_Edit/_Undo (Ctrl+Z)"
	menuitem = gtk_menu_item_new_with_label ("_Undo");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_edit_undo_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_z, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Edit/_Redo (Ctrl+Shift+Z)"
	menuitem = gtk_menu_item_new_with_label ("_Redo");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_edit_redo_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_z, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), separator);

	// ** Accel to "/_Edit/_Cut (Ctrl+X)"
	menuitem = gtk_menu_item_new_with_label ("_Cut");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_edit_cut_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_x, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Edit/_Copy (Ctrl+C)"
	menuitem = gtk_menu_item_new_with_label ("_Copy");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_edit_copy_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_c, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Edit/_Paste (Ctrl+V)"
	menuitem = gtk_menu_item_new_with_label ("_Paste");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_edit_paste_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_v, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), separator);

	// ** Accel to "/_Edit/_Delete"
	menuitem = gtk_menu_item_new_with_label ("_Delete");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_edit_delete_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_Delete, 0, GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

static GtkWidget *pv_get_menuitem_new_tree_of_layer_(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;
	GtkWidget *separator;

	menuitem_root = gtk_menu_item_new_with_label ("_Layer");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "/_Layer/_New Layer (Ctrl+Shift+N)"
	menuitem = gtk_menu_item_new_with_label ("_New Layer");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_layer_new_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_n, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Layer/_Copy Layer"
	menuitem = gtk_menu_item_new_with_label ("_Copy Layer");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_layer_copy_), NULL);

	// ** Accel to "/_Layer/New Child Layer"
	menuitem = gtk_menu_item_new_with_label ("New Child Layer");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_layer_new_child_), NULL);

	// ** Accel to "/_Layer/_Delete Layer"
	menuitem = gtk_menu_item_new_with_label ("_Delete Layer");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_layer_delete_), NULL);

	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), separator);

	// ** Accel to "/_Layer/_Raise(Ctrl+Shift+PageUp)"
	menuitem = gtk_menu_item_new_with_label ("_Raise");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_layer_raise_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_Page_Up, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Layer/_Lower(Ctrl+Shift+PageDown)"
	menuitem = gtk_menu_item_new_with_label ("_Lower");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_layer_lower_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_Page_Down, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Layer/Raise to _Top(Ctrl+Shift+Home)"
	menuitem = gtk_menu_item_new_with_label ("Raise to Top");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_layer_raise_to_top_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_Home, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Layer/Lower to _End(Ctrl+Shift+End)"
	menuitem = gtk_menu_item_new_with_label ("Lower to _End");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_layer_lower_to_end_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_End, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

static GtkWidget *pv_get_menuitem_new_tree_of_element_(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;
	GtkWidget *separator;

	menuitem_root = gtk_menu_item_new_with_label ("_Element");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "/_Element/_Raise(PageUp)"
	menuitem = gtk_menu_item_new_with_label ("_Raise");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_element_raise_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_Page_Up, (0), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Element/_Lower(PageDown)"
	menuitem = gtk_menu_item_new_with_label ("_Lower");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_element_lower_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_Page_Down, (0), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Element/Raise to _Top(Home)"
	menuitem = gtk_menu_item_new_with_label ("Raise to Top");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_element_raise_to_top_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_Home, (0), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Element/Lower to _End(End)"
	menuitem = gtk_menu_item_new_with_label ("Lower to _End");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_element_lower_to_end_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_End, (0), GTK_ACCEL_VISIBLE);

	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), separator);

	// ** Accel to "/_Element/_Grouping(Ctrl+G)"
	menuitem = gtk_menu_item_new_with_label ("_Grouping");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_element_grouping_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_g, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Element/Ungrouping(Ctrl+Shift+G)"
	menuitem = gtk_menu_item_new_with_label ("Ungrouping");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_element_ungrouping_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_g, (GDK_CONTROL_MASK | GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Element/Mask Grouping"
	menuitem = gtk_menu_item_new_with_label ("Mask Grouping");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_element_mask_grouping_), NULL);

	return menuitem_root;
}

static GtkWidget *pv_get_menuitem_new_tree_of_view_zoom_(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	menuitem_root = gtk_menu_item_new_with_label ("_Zoom");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "/_View/_Zoom/Zoom _In (Ctrl++)"
	menuitem = gtk_menu_item_new_with_label ("Zoom _In");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_view_zoom_in_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_plus, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_KP_Add, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE); // in ten key.

	// ** Accel to "/_View/_Zoom/Zoom _Out (Ctrl+-)"
	menuitem = gtk_menu_item_new_with_label ("Zoom _Out");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_view_zoom_out_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_minus, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_KP_Subtract, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE); // in ten key.

	return menuitem_root;
}

static GtkWidget *pv_get_menuitem_new_tree_of_view_(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	// menuitem_root = gtk_menu_item_new_with_mnemonic ("_View");
	menuitem_root = gtk_menu_item_new_with_mnemonic ("_View");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** "/_View/_Zoom/*"
	menuitem = pv_get_menuitem_new_tree_of_view_zoom_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

	// ** Accel to "View > extent show
	menuitem = gtk_check_menu_item_new_with_mnemonic ("_Extent View");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "toggled", G_CALLBACK(cb_menu_view_extent_), NULL);

	// ** Accel to "/_View/_Transparent Grid"
	menuitem = gtk_check_menu_item_new_with_mnemonic ("_Transparent Grid");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "toggled", G_CALLBACK(cb_menu_view_transparent_grid_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_d, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

static GtkWidget *pv_get_menuitem_new_tree_of_document_(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;
	GtkWidget *separator;

	menuitem_root = gtk_menu_item_new_with_mnemonic ("_Document");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "Document > Resize Document
	menuitem = gtk_menu_item_new_with_mnemonic ("_Resize Document");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_document_resize_), NULL);

	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), separator);

	// ** Accel to "/_Document/_Preferences"
	menuitem = gtk_menu_item_new_with_label ("_Preferences");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_document_preference_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_p, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

static GtkWidget *pv_get_menuitem_new_tree_of_tool_tool_(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	menuitem_root = gtk_menu_item_new_with_mnemonic ("_Tool");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	size_t num = et_tool_get_num();
	for(int i = 0; i < (int)num; i++){
		const EtToolInfo *info = et_tool_get_info_from_id(i);
		et_assertf(info, "%d/%zu", i, num);

		menuitem = gtk_menu_item_new_with_mnemonic(info->name);
		gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
		g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_tool_change_), (gpointer)info);

		int t = 0;
		while(0 != info->shortcuts[t].accel_key){
			gtk_widget_add_accelerator (menuitem, "activate", accel_group,
					info->shortcuts[t].accel_key,
					info->shortcuts[t].accel_mods,
					GTK_ACCEL_VISIBLE);
			t++;
		}
	}

	return menuitem_root;
}

static GtkWidget *pv_get_menuitem_new_tree_of_tool_(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;
	GtkWidget *separator;

	menuitem_root = gtk_menu_item_new_with_mnemonic ("_Tool");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** "/_Tool/_Tool/*"
	menuitem = pv_get_menuitem_new_tree_of_tool_tool_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), separator);

	// ** Accel to "/_Tool/Snap for grid(Ctrl+Shift+\)"
	menuitem = gtk_check_menu_item_new_with_label ("Snap for grid");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "toggled", G_CALLBACK(cb_menu_tool_snap_for_grid_), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_backslash, (GDK_CONTROL_MASK | GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_bar, (GDK_CONTROL_MASK | GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
	self->menuitem_snap_for_grid = menuitem;

	return menuitem_root;
}

static GtkWidget *pv_get_menuitem_new_tree_of_help_(GtkAccelGroup *accel_group){
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	menuitem_root = gtk_menu_item_new_with_mnemonic ("_Help");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "Help > About"
	menuitem = gtk_menu_item_new_with_mnemonic ("_About");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(cb_menu_help_about_), NULL);
	/*
	   gtk_widget_add_accelerator (menuitem, "activate", accel_group,
	   GDK_KEY_a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	 */

	return menuitem_root;
}

// ** Issue: Mnemonic not works on submenu in Ubuntu15.10(cause Unity/Ubuntu?).
bool init_menu(EtWindow *et_window, GtkWidget *box_root)
{
	et_assert(NULL == et_window_);
	et_window_ = et_window;
	GtkWindow *window = et_window_->window;

	GtkWidget *menubar;
	GtkWidget *menuitem;

	GtkAccelGroup *accel_group;
	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	menubar = gtk_menu_bar_new ();
	gtk_box_pack_start (GTK_BOX (box_root), menubar, FALSE, TRUE, 0);

	menuitem = pv_get_menuitem_new_tree_of_file_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = pv_get_menuitem_new_tree_of_edit_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = pv_get_menuitem_new_tree_of_layer_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = pv_get_menuitem_new_tree_of_element_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = pv_get_menuitem_new_tree_of_select_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = pv_get_menuitem_new_tree_of_view_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = pv_get_menuitem_new_tree_of_document_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = pv_get_menuitem_new_tree_of_tool_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = pv_get_menuitem_new_tree_of_help_(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	return true;
}

void slot_et_menu_from_etaion_change_state(EtState state, gpointer data)
{
	if(0 > state.doc_id){
		return;
	}

	PvDocumentPreference document_preference = et_doc_get_document_preference_from_id(state.doc_id);
	gtk_check_menu_item_set_active(
			GTK_CHECK_MENU_ITEM(self->menuitem_snap_for_grid),
			document_preference.snap_context.is_snap_for_grid);
}

