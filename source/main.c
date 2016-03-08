#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <stdbool.h>
#include <stdatomic.h>

#include "et_error.h"
#include "et_canvas.h"
#include "et_canvas_collection.h"
#include "et_thumbnail.h"
#include "et_doc.h"
#include "et_renderer.h"
#include "et_pointing_manager.h"
#include "et_doc_manager.h"
#include "et_etaion.h"
#include "et_layer_view.h"
#include "pv_io.h"

const char *APP_NAME = "Etaion Vector Graphic Editor";
GtkWindow *_main_window = NULL;

void _pvui_app_set_style();
bool _init_menu(GtkWidget *window, GtkWidget *box_root);
bool _debug_init();
EtDocId _open_doc_new(PvVg *pv_src);
EtDocId _open_doc_new_from_file(const char* filepath);



GtkWidget *status_bar= NULL;
static gboolean in_worker_func(gpointer data)
{
	static int count = 0;
	count++;
	// Gtk control.
	printf("work:%d\n", count);
	char s[64];
	snprintf(s, sizeof(s), "%d", count);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), 1, s);

	return G_SOURCE_REMOVE;
}
static atomic_int isStop = ATOMIC_VAR_INIT(0);
static gpointer worker_func(gpointer data)
{
	while( !isStop ){
		g_usleep(500000);
		gdk_threads_add_idle(in_worker_func, NULL);
	}

	return NULL;
}

static gboolean cb_key_press(GtkWidget *widget, GdkEventKey * event, gpointer user_data)
{
	g_print("keyval=%d static=%d string=%s\n",
			event->keyval, event->state, event->string);

	EtKeyAction ka = {
		.key = event->keyval,
		.action = EtKeyAction_Down,
	};
	et_etaion_slot_key_action(ka);

	return FALSE;
}

int main (int argc, char **argv){
	gtk_init(&argc, &argv);


	// ** The etaion core modules initialize.
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
	_main_window = GTK_WINDOW(window);

	GtkWidget *box_root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_container_add(GTK_CONTAINER(window), box_root);

	if(! _init_menu(window, box_root)){
		et_bug("");
		return -1;
	}

	g_signal_connect(G_OBJECT(window), "key-press-event",
			G_CALLBACK(cb_key_press), NULL);
	g_signal_connect(window, "delete-event",
			G_CALLBACK(gtk_main_quit), NULL);

	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_container_add(GTK_CONTAINER(box_root), vbox);
	GtkWidget *box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_pack_start(GTK_BOX(vbox), box1, true, true, 1);

	GtkWidget *statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, TRUE, 0);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), 1, "Hello World");
	status_bar = statusbar;

	GtkWidget *hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(box1), hpaned, true, true, 3);

	GtkWidget *box_dock_work = gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);
	gtk_paned_pack2 (GTK_PANED (hpaned), box_dock_work, FALSE, FALSE);


	// ** The etaion gui modules initialize.
	EtCanvasCollection *canvas_collection = et_canvas_collection_init();
	if(NULL == canvas_collection){
		et_bug("");
		return -1;
	}
	GtkWidget *cancol_widget = et_canvas_collection_get_widget_frame();
	gtk_paned_pack1 (GTK_PANED (hpaned), cancol_widget, TRUE, FALSE);


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
			true, true, 3);

	EtPointingManager *pointing_manager = et_pointing_manager_init();
	if(NULL == pointing_manager){
		et_error("");
		return -1;
	}

	if(!et_pointing_manager_set_slot_mouse_action(
				et_etaion_slot_mouse_action)){
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
	thread = g_thread_new ("", worker_func, NULL);
	if(NULL == thread){
		et_critical("");
		return -1;
	}

	gtk_widget_show_all(window);
	gtk_main();

	isStop = 1;
	g_thread_join(thread);

	return 0;
}

bool _debug_init()
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

EtDocId _open_doc_new_from_file(const char* filepath)
{
	et_debug("filepath:'%s'\n", (NULL == filepath)? "NULL":filepath);

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

EtDocId _open_doc_new(PvVg *vg_src)
{

	EtDocId doc_id = et_doc_manager_new_doc();
	if(0 > doc_id){
		et_error("");
		return -1;
	}

	// ** setting doc
	if(NULL != vg_src){
		PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
		if(NULL == vg){
			et_error("");
			return -1;
		}
		if(!pv_vg_copy_overwrite(vg, vg_src)){
			et_error("");
			return -1;
		}
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

	if(!et_layer_view_set_doc_id(doc_id)){
		et_error("");
		return -1;
	}

	return doc_id;
}

void _pvui_app_set_style(){
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
			"GtkWindow {\n"
			"   background-color: rgb (103, 103, 103);\n"
			"}\n"
			//"GtkWidget {\n"
			"#canvasGtk {\n"
			"   background-color: rgb (50, 50, 50);\n"
			"   border-style: solid;\n"
			"   border-width: 1px;\n"
			"   border-color: rgb (255, 255, 10);\n"
			"   margin: 0px;\n"
			"   padding: 0px;\n"
			"}\n"
			"#canvasNanoSVG {\n"
			"   background-color: rgb (50, 50, 50);\n"
			"   border-style: solid;\n"
			"   border-width: 1px;\n"
			"   border-color: rgb (10, 255, 255);\n"
			"   margin: 0px;\n"
			"   padding: 0px;\n"
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
				et_debug("size:%f,%f,%f,%f\n",
						vg->rect.x, vg->rect.y, vg->rect.w, vg->rect.h);
				_open_doc_new(vg);
			}
			break;
		default:
			et_debug("Cancel\n");
			break;
	}
	gtk_widget_destroy (dialog);

	pv_vg_free(vg);

	return false;
}

bool _save_file_from_doc_id(const char *filepath, EtDocId doc_id)
{
	if(NULL == filepath){
		et_bug("");
		goto error;
	}


	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_debug("%d\n", doc_id);
		goto error;
	}

	PvVg *vg = et_doc_get_vg_ref(doc);
	if(NULL == vg){
		et_debug("%d\n", doc_id);
		goto error;
	}

	if(!pv_io_write_file_svg_from_vg(vg, filepath)){
		et_debug("%d\n", doc_id);
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
				_main_window,
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
			et_debug("file not open:%s\n", filename);
		}
		g_free (filename);
	}

	gtk_widget_destroy (dialog);



	return false;
}

void _cb_menu_view_extent(GtkCheckMenuItem *menuitem, gpointer user_data)
{
	if(!et_etaion_set_is_extent_view(gtk_check_menu_item_get_active(menuitem))){
		et_error("");
	}
}

void _cb_menu_help_about (GtkMenuItem *menuitem, gpointer user_data)
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

GtkWidget *pv_get_menuitem_new_tree_of_file(GtkAccelGroup *accel_group){
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

GtkWidget *_pv_get_menuitem_new_tree_of_view(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	// menuitem_root = gtk_menu_item_new_with_mnemonic ("_View");
	menuitem_root = gtk_menu_item_new_with_mnemonic ("_View");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

#define GDK_ALT_MASK (GDK_MOD1_MASK)

	// ** Accel to "View > extent show
	menuitem = gtk_check_menu_item_new_with_mnemonic ("_Extent View");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "toggled", G_CALLBACK(_cb_menu_view_extent), NULL);
	//gtk_widget_add_accelerator (menuitem, "toggled", accel_group,
	//		GDK_KEY_e,( GDK_ALT_MASK|GDK_SHIFT_MASK|GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

GtkWidget *_new_tree_of_help(GtkAccelGroup *accel_group){
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	menuitem_root = gtk_menu_item_new_with_mnemonic ("_Help");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "Help > About (Ctrl+A)"
	menuitem = gtk_menu_item_new_with_mnemonic ("_About");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_help_about), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

// ** Issue: Mnemonic not works on submenu in Ubuntu15.10(cause Unity/Ubuntu?).
bool _init_menu(GtkWidget *window, GtkWidget *box_root)
{
	GtkWidget *menubar;
	GtkWidget *menuitem;

	GtkAccelGroup *accel_group;
	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	menubar = gtk_menu_bar_new ();
	gtk_box_pack_start (GTK_BOX (box_root), menubar, FALSE, TRUE, 0);

	menuitem = pv_get_menuitem_new_tree_of_file(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = _pv_get_menuitem_new_tree_of_view(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = _new_tree_of_help(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	return true;
}

