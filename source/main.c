#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <stdbool.h>
#include <stdatomic.h>
#include <stdlib.h>

#include "et_error.h"
#include "et_define.h"
#include "et_canvas.h"
#include "et_canvas_collection.h"
#include "et_thumbnail.h"
#include "et_doc.h"
#include "et_renderer.h"
#include "et_pointing_manager.h"
#include "et_doc_manager.h"
#include "et_etaion.h"
#include "et_layer_view.h"
#include "et_tool_panel.h"
#include "pv_io.h"
#include "et_color_panel.h"
#include "et_stroke_panel.h"
#include "et_position_panel.h"
#include "et_snap_panel.h"

const char *APP_NAME = "Etaion Vector Graphic Editor";

typedef struct{
	GtkWindow *window;
	GtkWidget *status_bar;
}EtWindow;

static EtWindow window_;
static EtWindow *self = &window_;

static void _pvui_app_set_style();
static bool _init_menu(GtkWidget *window, GtkWidget *box_root);
static bool _debug_init();
static EtDocId _open_doc_new(PvVg *pv_src);
static EtDocId _open_doc_new_from_file(const char* filepath);



static bool _slot_mouse_action(EtDocId, EtMouseAction);



static gboolean in_worker_func(gpointer data)
{
	static int count = 0;
	count++;
	// Gtk control.
	printf("work:%d\n", count);

	return G_SOURCE_REMOVE;
}
static atomic_int isStop = ATOMIC_VAR_INIT(0);
static gpointer _worker_func(gpointer data)
{
	while( !isStop ){
		g_usleep(500000);
		gdk_threads_add_idle(in_worker_func, NULL);
	}

	return NULL;
}

static gboolean _cb_key_press(GtkWidget *widget, GdkEventKey * event, gpointer user_data)
{
	et_debug("keyval=%04x status=%04x",
			event->keyval, event->state/*, event->string*/);

	EtKeyAction ka = {
		.key = event->keyval,
		.action = EtKeyAction_Down,
		.state = event->state,
	};
	et_etaion_slot_key_action(ka);

	return FALSE;
}

int main (int argc, char **argv){
	gtk_init(&argc, &argv);


	// ** The etaion core modules initialize.
	if(!et_tool_init()){
		et_bug("");
		return -1;
	}
	EtEtaion *current_state = et_etaion_init();
	if(NULL == current_state){
		et_bug("");
		return -1;
	}

	EtDocManager *doc_manager = et_doc_manager_init();
	if(NULL == doc_manager){
		et_error("");
		return -1;
	}

	EtRenderer *renderer = et_renderer_init();
	if(NULL == renderer){
		et_error("");
		return -1;
	}


	// ** window and container(box)s application base.
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request (window, 900,700);
	self->window = GTK_WINDOW(window);

	GtkWidget *box_root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_container_add(GTK_CONTAINER(window), box_root);

	if(! _init_menu(window, box_root)){
		et_bug("");
		return -1;
	}

	g_signal_connect(G_OBJECT(window), "key-press-event",
			G_CALLBACK(_cb_key_press), NULL);
	g_signal_connect(window, "delete-event",
			G_CALLBACK(gtk_main_quit), NULL);

	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_container_add(GTK_CONTAINER(box_root), vbox);

	GtkWidget *box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_pack_start(GTK_BOX(vbox), box1, true, true, 1);

	GtkWidget *box_underbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_pack_start(GTK_BOX(vbox), box_underbar, false, false, 1);

	GtkWidget *statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, TRUE, 0);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), 1, "-,(-)");
	self->status_bar = statusbar;

	GtkWidget *hbox_left = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_pack_start(GTK_BOX(box1), hbox_left, false, false, 3);

	GtkWidget *hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(box1), hpaned, true, true, 3);

	GtkWidget *box_dock_work = gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);
	gtk_paned_pack2 (GTK_PANED (hpaned), box_dock_work, FALSE, FALSE);


	// ** The etaion gui modules initialize.
	EtToolPanel *tool_panel = et_tool_panel_init();
	if(NULL == tool_panel){
		et_bug("");
		return -1;
	}
	GtkWidget *toolpanel_widget = et_tool_panel_get_widget_frame();
	gtk_box_pack_start(GTK_BOX(hbox_left), toolpanel_widget, false, false, 3);

	if(!et_tool_panel_set_slot_change(slot_et_etaion_change_tool, NULL)){
		et_bug("");
		return -1;
	}

	EtCanvasCollection *canvas_collection = et_canvas_collection_init();
	if(NULL == canvas_collection){
		et_bug("");
		return -1;
	}
	GtkWidget *cancol_widget = et_canvas_collection_get_widget_frame();
	gtk_paned_pack1 (GTK_PANED (hpaned), cancol_widget, TRUE, FALSE);


	EtColorPanel *color_panel = et_color_panel_init();
	if(NULL == color_panel){
		et_bug("");
		return -1;
	}
	if(0 > et_etaion_set_slot_change_state(
				slot_et_color_panel_from_etaion_change_state, NULL)){
		et_bug("");
		return -1;
	}
	gtk_box_pack_start(GTK_BOX(box_dock_work),
			et_color_panel_get_widget_frame(),
			false, false, 3);

	EtStrokePanel *stroke_panel = et_stroke_panel_init();
	if(NULL == stroke_panel){
		et_bug("");
		return -1;
	}
	if(0 > et_etaion_set_slot_change_state(
				slot_et_stroke_panel_from_etaion_change_state, NULL)){
		et_bug("");
		return -1;
	}
	gtk_box_pack_start(GTK_BOX(box_dock_work),
			et_stroke_panel_get_widget_frame(),
			false, false, 3);

	EtPositionPanel *position_panel = et_position_panel_init();
	assert(position_panel);
	if(0 > et_etaion_set_slot_change_state(
				slot_et_position_panel_from_etaion_change_state, NULL)){
		et_bug("");
		return -1;
	}
	gtk_box_pack_start(GTK_BOX(box_underbar),
			et_position_panel_get_widget_frame(),
			true, true, 3);

	EtSnapPanel *snap_panel = et_snap_panel_init();
	assert(snap_panel);
	if(0 > et_etaion_set_slot_change_state(
				slot_et_snap_panel_from_etaion_change_state, NULL)){
		et_bug("");
		return -1;
	}
	gtk_box_pack_start(GTK_BOX(box_underbar),
			et_snap_panel_get_widget_frame(),
			true, true, 3);

	EtLayerView *layer_view = et_layer_view_init();
	if(NULL == layer_view){
		et_bug("");
		return -1;
	}
	if(0 > et_etaion_set_slot_change_state(
				et_layer_view_slot_from_etaion_change_state, NULL)){
		et_bug("");
		return -1;
	}
	gtk_box_pack_start(GTK_BOX(box_dock_work),
			et_layer_view_get_widget_frame(),
			false, true, 3);

	EtPointingManager *pointing_manager = et_pointing_manager_init();
	if(NULL == pointing_manager){
		et_error("");
		return -1;
	}

	if(!et_pointing_manager_add_slot_mouse_action(
				et_etaion_slot_mouse_action)){
		et_error("");
		return -1;
	}

	if(!et_pointing_manager_add_slot_mouse_action(
				_slot_mouse_action)){
		et_error("");
		return -1;
	}

	GtkWidget *frame_thumb_canvas = gtk_frame_new (NULL);
	gtk_frame_set_label(GTK_FRAME (frame_thumb_canvas), "Thumbnail");
	gtk_frame_set_shadow_type (GTK_FRAME (frame_thumb_canvas), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(box_dock_work), frame_thumb_canvas,
			false, true, 3);

	EtThumbnail *thumbnail = et_canvas_collection_get_thumbnail();
	EtCanvas *canvas_thumbnail = et_thumbnail_get_canvas(thumbnail);
	GtkWidget *canvas_thumbnail_widget = et_canvas_get_widget_frame(canvas_thumbnail);
	if(NULL == canvas_thumbnail_widget){
		et_bug("");
		return -1;
	}
	gtk_container_add(GTK_CONTAINER(frame_thumb_canvas), canvas_thumbnail_widget);


	if(0 > et_canvas_set_slot_mouse_action(canvas_thumbnail,
				et_pointing_manager_slot_mouse_action, NULL)){
		et_error("");
		return -1;
	}



	if(!_debug_init()){
		et_error("");
		return -1;
	}



	GThread* thread;
	thread = g_thread_new ("", _worker_func, NULL);
	if(NULL == thread){
		et_critical("");
		return -1;
	}

	_pvui_app_set_style();
	gtk_widget_show_all(window);
	gtk_main();

	isStop = 1;
	g_thread_join(thread);

	return 0;
}

static bool _debug_init()
{
	EtDocId doc_id = _open_doc_new(NULL);
	if(0 > doc_id){
		et_error("");
		return -1;
	}

	PvVg *vg = et_doc_get_vg_ref(et_doc_manager_get_doc_from_id(doc_id));
	if(NULL == vg){
		et_error("");
		return -1;
	}
	vg->rect.w = 600;
	vg->rect.h = 500;

	// ** (開発デバッグ用)ドキュメントにsvgを貼り付ける
	const char *pathImageFile = NULL;
	pathImageFile = "./library/23.svg";
	if(!et_doc_set_image_from_file(et_doc_manager_get_doc_from_id(doc_id), pathImageFile)){
		et_error("");
		return false;
	}

	if(!et_doc_signal_update_from_id(doc_id)){
		et_error("");
		return false;
	}

	return true;
}

static EtDocId _open_doc_new_from_file(const char* filepath)
{
	et_debug("filepath:'%s'", (NULL == filepath)? "NULL":filepath);

	if(NULL == filepath){
		et_error("");
		return -1;
	}

	PvVg *vg_src = pv_io_new_from_file(filepath);
	if(NULL == vg_src){
		et_error("");
		return -1;
	}

	EtDocId doc_id = _open_doc_new(vg_src);
	if(0 > doc_id){
		et_error("");
		return -1;
	}

	pv_vg_free(vg_src);

	et_doc_signal_update_from_id(doc_id);

	return doc_id;
}

static EtDocId _open_doc_new(PvVg *vg_src)
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

	if(!et_layer_view_set_doc_id(doc_id)){
		et_error("");
		return -1;
	}

	return doc_id;
}

static void _pvui_app_set_style(){
	GtkCssProvider *provider;
	provider = gtk_css_provider_new ();

	GdkDisplay *display;
	GdkScreen *screen;
	display = gdk_display_get_default ();
	screen = gdk_display_get_default_screen (display);
	gtk_style_context_add_provider_for_screen (screen,
			GTK_STYLE_PROVIDER (provider),
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	gtk_css_provider_load_from_data (GTK_CSS_PROVIDER(provider),
			/*
			   "GtkWindow {\n"
			   "   background-color: rgb (103, 103, 103);\n"
			   "}\n"
			   "GtkWidget {\n"
			   "   background-color: rgb (103, 103, 103);\n"
			   "}\n"
			 */
			"GtkNotebook {\n"
			"   padding: 2px;\n"
			"}\n"
			"GtkFrame {\n"
			"   padding: 2px;\n"
			"}\n"
			"#toolpanel {\n"
			"   border-color: rgb (100, 100, 10);\n"
			"   border-width: 2px;\n"
			"   padding: 2px;\n" 
			"}\n"
			"GtkDrawingArea {\n"
			"   background-color: rgb(255, 255, 255);\n"
			//"   background-color: rgb(0, 0, 0);\n"
			"}\n"
			"GtkStatusbar {\n"
			"   font: monospace;\n"
			"}\n"
			"", -1, NULL);
	g_object_unref (provider);
}

static gboolean _cb_menu_file_new(gpointer data)
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

	GtkWidget *dialog;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_dialog_new_with_buttons ("New Document",
			NULL,
			flags,
			"_OK",
			GTK_RESPONSE_ACCEPT,
			"_Cancel",
			GTK_RESPONSE_REJECT,
			NULL);

	GtkWidget *hbox_w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	GtkWidget *hbox_h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	GtkWidget *spin_w = gtk_spin_button_new_with_range(0, 20000, 100);
	GtkWidget *spin_h = gtk_spin_button_new_with_range(0, 20000, 100);
	GtkWidget *label_w = gtk_label_new_with_mnemonic("width ");
	GtkWidget *label_h = gtk_label_new_with_mnemonic("height");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_w), 1200);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_h), 600);
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
				et_debug("size:%f,%f,%f,%f",
						vg->rect.x, vg->rect.y, vg->rect.w, vg->rect.h);
				_open_doc_new(vg);
			}
			break;
		default:
			et_debug("Cancel");
			break;
	}
	gtk_widget_destroy (dialog);

	pv_vg_free(vg);

	return false;
}

static bool _save_file_from_doc_id(const char *filepath, EtDocId doc_id)
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

static gboolean _cb_menu_file_save(gpointer data)
{

	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// Todo: Nothing document.
		et_bug("%d\n", doc_id);
		return false;
	}

	char *filepath = NULL;
	if(!et_doc_get_filepath(&filepath, doc_id)){
		et_error("");
		goto error;
	}

	if(NULL == filepath){
		GtkWidget *dialog;
		GtkFileChooser *chooser;
		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
		gint res;

		dialog = gtk_file_chooser_dialog_new ("Save File",
				self->window,
				action,
				_("_Cancel"),
				GTK_RESPONSE_CANCEL,
				_("_Save"),
				GTK_RESPONSE_ACCEPT,
				NULL);
		chooser = GTK_FILE_CHOOSER (dialog);

		gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

		gtk_file_chooser_set_current_name (chooser,
				_("Untitled_document.svg"));

		res = gtk_dialog_run (GTK_DIALOG (dialog));
		if (res == GTK_RESPONSE_ACCEPT)
		{
			filepath = gtk_file_chooser_get_filename (chooser);
		}

		gtk_widget_destroy (dialog);
	}

	if(NULL != filepath){
		if(!et_doc_set_filepath(doc_id, filepath)){
			// TODO: error dialog.
			et_error("");
			goto error;
		}
		if(!_save_file_from_doc_id(filepath, doc_id)){
			// TODO: error dialog.
			et_error("");
			goto error;
		}
	}

error:
	g_free (filepath);
	return false;
}

static gboolean _cb_menu_file_open(gpointer data)
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

	gint res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		filename = gtk_file_chooser_get_filename (chooser);
		if(!_open_doc_new_from_file(filename)){
			// TODO: warning dialog.
			et_debug("file not open:'%s'", filename);
		}
		g_free (filename);
	}

	gtk_widget_destroy (dialog);



	return false;
}

static void _cb_menu_view_extent(GtkCheckMenuItem *menuitem, gpointer user_data)
{
	if(!et_etaion_set_is_extent_view(gtk_check_menu_item_get_active(menuitem))){
		et_error("");
	}
}

static bool _pv_element_is_exist_from_elements(const PvElement *element, PvElement **elements)
{
	int num = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < num; i++){
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

static bool _cb_menu_select_all_func_recurse_prev(PvElement *element, gpointer data, int level)
{
	EtSelectAllFuncRecurseDataPack *func_safr_data_pack = data;
	PvFocus *focus = func_safr_data_pack->focus;

	if(PvElementKind_Layer == element->kind){
		return true;
	}
	if(_pv_element_is_exist_from_elements(element, func_safr_data_pack->elements_ignore)){
		return true;
	}

	if(!pv_focus_add_element(focus, element)){
		return false;
	}

	return true;
}

static void _cb_menu_select_all (GtkMenuItem *menuitem, gpointer user_data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// Todo: Nothing document.
		et_bug("%d\n", doc_id);
		return;
	}

	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	if(NULL == vg){
		et_debug("%d", doc_id);
		return;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
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
				_cb_menu_select_all_func_recurse_prev,
				&func_safr_data_pack,
				&error))
	{
		et_error("level:%d", error.level);
		return;
	}

	et_doc_signal_update_from_id(doc_id);
}

static void _cb_menu_select_none (GtkMenuItem *menuitem, gpointer user_data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// Todo: Nothing document.
		et_bug("%d\n", doc_id);
		return;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		et_bug("");
		return;
	}

	if(!pv_focus_clear_to_parent_layer(focus)){
		et_error("");
		return;
	}

	et_doc_signal_update_from_id(doc_id);
}

static void _cb_menu_select_invert (GtkMenuItem *menuitem, gpointer user_data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		// Todo: Nothing document.
		et_bug("%d\n", doc_id);
		return;
	}

	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	if(NULL == vg){
		et_debug("%d", doc_id);
		return;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		et_bug("");
		return;
	}

	int num = pv_general_get_parray_num((void **)focus->elements);
	PvElement **elements_prefocus = malloc(sizeof(PvElement *) * (num + 1));
	if(NULL == elements_prefocus){
		et_bug("");
		return;
	}
	memcpy(elements_prefocus, focus->elements, sizeof(PvElement *) * (num + 1));

	if(!pv_focus_clear_to_parent_layer(focus)){
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
				_cb_menu_select_all_func_recurse_prev,
				&func_safr_data_pack,
				&error))
	{
		et_error("level:%d", error.level);
		goto finally;
	}

finally:
	free(elements_prefocus);

	et_doc_signal_update_from_id(doc_id);
}

static void _cb_menu_help_about (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWindow *parent_window = NULL;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *dialog = gtk_message_dialog_new (parent_window,
			flags,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_CLOSE,
			"This is :'%s'",
			APP_NAME);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

static GtkWidget *_pv_get_menuitem_new_tree_of_select(GtkAccelGroup *accel_group){
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
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_select_all), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	// ** Accel to "/_Select/_None (Shift+Ctrl+A)"
	menuitem = gtk_menu_item_new_with_label ("_None");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_select_none), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_a, (GDK_SHIFT_MASK | GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);
	// ** Accel to "/_Select/_Invert (Ctrl+I)"
	menuitem = gtk_menu_item_new_with_label ("_Invert");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_select_invert), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

static GtkWidget *_pv_get_menuitem_new_tree_of_file(GtkAccelGroup *accel_group){
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
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_file_new), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// ** "/_File/_Open (Ctrl+O)"
	menuitem = gtk_menu_item_new_with_label ("_Open");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_file_open), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// ** "/_File/_Save (Ctrl+S)"
	menuitem = gtk_menu_item_new_with_label ("_Save");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_file_save), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	/*
	   menuitem = gtk_menu_item_new_with_label ("Save As");
	   gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	   menuitem = pv_get_menuitem_new_tree_of_export();
	   gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	 */
	menuitem = gtk_menu_item_new_with_label ("_Quit");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(gtk_main_quit), NULL);

	return menuitem_root;
}

static GtkWidget *_pv_get_menuitem_new_tree_of_view(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	// menuitem_root = gtk_menu_item_new_with_mnemonic ("_View");
	menuitem_root = gtk_menu_item_new_with_mnemonic ("_View");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "View > extent show
	menuitem = gtk_check_menu_item_new_with_mnemonic ("_Extent View");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "toggled", G_CALLBACK(_cb_menu_view_extent), NULL);
	//gtk_widget_add_accelerator (menuitem, "toggled", accel_group,
	//		GDK_KEY_e,( ET_GDK_ALT_MASK|GDK_SHIFT_MASK|GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

static GtkWidget *_new_tree_of_help(GtkAccelGroup *accel_group){
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
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_help_about), NULL);
	/*
	   gtk_widget_add_accelerator (menuitem, "activate", accel_group,
	   GDK_KEY_a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	 */

	return menuitem_root;
}

// ** Issue: Mnemonic not works on submenu in Ubuntu15.10(cause Unity/Ubuntu?).
static bool _init_menu(GtkWidget *window, GtkWidget *box_root)
{
	GtkWidget *menubar;
	GtkWidget *menuitem;

	GtkAccelGroup *accel_group;
	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	menubar = gtk_menu_bar_new ();
	gtk_box_pack_start (GTK_BOX (box_root), menubar, FALSE, TRUE, 0);

	menuitem = _pv_get_menuitem_new_tree_of_file(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = _pv_get_menuitem_new_tree_of_select(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = _pv_get_menuitem_new_tree_of_view(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = _new_tree_of_help(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	return true;
}

static bool _slot_mouse_action(EtDocId doc_id, EtMouseAction mouse_action)
{
	assert(self);

	char s[256];
	snprintf(s, sizeof(s), "%7.2f,%7.2f(%6.1f,%6.1f)",
			mouse_action.point.x,
			mouse_action.point.y,
			mouse_action.raw.x,
			mouse_action.raw.y);
	gtk_statusbar_push(GTK_STATUSBAR(self->status_bar), 1, s);

	return true;
}

