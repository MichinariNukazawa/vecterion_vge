#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <stdbool.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
#include "et_tool_info.h"
#include "et_tool_panel.h"
#include "pv_io.h"
#include "pv_renderer.h"
#include "et_color_panel.h"
#include "et_stroke_panel.h"
#include "et_position_panel.h"
#include "pv_file_format.h"
#include "et_doc_relation.h"
#include "version.h"

#define VECTERION_FULLNAME "Vecterion Vector Graphic Editor"



typedef struct{
	GtkWindow *window;
	GtkWidget *status_bar;
}EtWindow;

static EtWindow window_;
static EtWindow *self = &window_;

static void _pvui_app_set_style();
static bool _init_menu(GtkWidget *window, GtkWidget *box_root);
// static bool _debug_init();
static EtDocId _open_doc_new(PvVg *pv_src);
static EtDocId _open_doc_new_from_file(const char* filepath);
static bool _output_file_from_doc_id(const char *filepath, EtDocId doc_id);
static bool _output_svg_from_doc_id(const char *filepath, EtDocId doc_id);



static bool slot_from_mouse_action_(EtDocId, EtMouseAction);

#define _show_error_dialog(fmt, ...) \
	do{ \
		et_error(""fmt"", ## __VA_ARGS__); \
		char *message = g_strdup_printf(""fmt"", ## __VA_ARGS__ ); \
		_show_error_dialog_impl(message); \
		g_free(message); \
	}while(0)
static void _show_error_dialog_impl(const char *message);


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
	slot_et_etaion_from_key_action(ka);

	return FALSE;
}

static bool _gui_quit();

static gboolean _cb_delete_event(GtkWidget *widget,
		GdkEvent  *event,
		gpointer   user_data)
{
	return !_gui_quit();
}


typedef struct{
	const char *input_filepath;
	const char *output_filepath;
}EtArgs;

EtArgs EtArgs_Default = {
	NULL,
	NULL,
};

static void usage()
{
	fprintf(stderr,
			"usage: %s [-i input_filepath [-o output_filepath]]\n"
			" -i input_filepath		open file.\n"
			" 				default if svg format is extract pvvg structure in memory.\n"
			" -o output_filepath		output file and exit this app.\n"
			" 				depend -i option.\n"
			" 				file format from filename extension.\n",
			"vecterion"
	       );
	fprintf(stderr, "output_filepath format(extension):\n");

	for(int i = 0; i < (int)get_num_file_formats(); i++){
		const PvFileFormat *format = get_file_format_from_index(i);
		fprintf(stderr, "	'%s'\n", format->extension);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "`%s`\n%s", VECTERION_FULLNAME, get_vecterion_build());
}

static bool et_args(EtArgs *args, int argc, char **argv)
{
	extern char *optarg;
	extern int optind, opterr, optopt;

	int opt;
	while (-1 != (opt = getopt(argc, argv, "i:o:h"))) {
		switch (opt) {
			case 'i':
				args->input_filepath = optarg;
				break;
			case 'o':
				args->output_filepath = optarg;
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
				break;
			case '?':
				et_error("'%c'", (char)optopt);
				return false;
				break;
			default:
				et_error("");
				return false;
		}
	}

	if(NULL != args->input_filepath){
		// ** exist input_filepath
		struct stat sb;

		errno = 0;
		if(-1 == stat(args->input_filepath, &sb)){
			fprintf(stderr, "input_filepath is not exist or other. %m:'%s'",
					args->input_filepath);
			return false;
		}
		if(! S_ISREG(sb.st_mode)){
			fprintf(stderr, "input_filepath is not normal file.:'%s'",
					args->input_filepath);
			et_error("");
			return false;
		}

		// ** check input_filepath format from extension
		const PvFileFormat *format = get_file_format_from_filepath(args->input_filepath);
		if(!format){
			fprintf(stderr, "input_filepath is invalid file type.:'%s'",
					args->input_filepath);
			return false;
		}
	}


	bool ret = true;
	if(NULL != args->output_filepath){
		struct stat sb;

		// ** output need input
		if(NULL == args->input_filepath && NULL != args->output_filepath){
			fprintf(stderr, "output_filepath depend input_filepath.");
			return false;
		}
		// ** output_filepath is normal file if exist
		if((0 == stat(args->output_filepath, &sb)) && (! S_ISREG(sb.st_mode))){
			fprintf(stderr, "output_filepath is not normal file. 0x%08x:'%s'",
					sb.st_mode, args->output_filepath);
			return false;
		}

		// ** output_filepath filetype from extension
		const PvFileFormat *format = get_file_format_from_filepath(args->output_filepath);
		if(!format){
			fprintf(stderr, "output_filepath is invalid file type.:'%s'",
					args->output_filepath);
			return false;
		}

		// ** check output directory exist
		char *output_dirpath = g_path_get_dirname(args->output_filepath);
		if((-1 == stat(output_dirpath, &sb)) || (! S_ISDIR(sb.st_mode))){
			fprintf(stderr, "output_filepath directory is not exist. '%s'",
					args->output_filepath);
			ret = false;
		}
		g_free(output_dirpath);
	}

	return ret;
}



int main (int argc, char **argv){
	gtk_init(&argc, &argv);

	EtArgs args_ = EtArgs_Default;
	EtArgs *args = &args_;
	if(!et_args(args, argc, argv)){
		usage();
		exit(EXIT_SUCCESS);
	}

	// ** The etaion core modules initialize.
	if(!et_tool_info_init()){
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
			G_CALLBACK(_cb_delete_event), NULL);

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

	if(!et_tool_panel_set_slot_change(
				slot_et_etaion_change_tool, NULL)){
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
	if(0 > et_etaion_add_slot_change_state(
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
	if(0 > et_etaion_add_slot_change_state(
				slot_et_stroke_panel_from_etaion_change_state, NULL)){
		et_bug("");
		return -1;
	}
	gtk_box_pack_start(GTK_BOX(box_dock_work),
			et_stroke_panel_get_widget_frame(),
			false, false, 3);

	EtPositionPanel *position_panel = et_position_panel_init();
	assert(position_panel);
	if(0 > et_etaion_add_slot_change_state(
				slot_et_position_panel_from_etaion_change_state, NULL)){
		et_bug("");
		return -1;
	}
	if(0 > et_etaion_add_slot_change_tool_id(
				slot_et_position_panel_from_etaion_change_tool_id, NULL)){
		et_bug("");
		return -1;
	}
	gtk_box_pack_start(GTK_BOX(box_underbar),
			et_position_panel_get_widget_frame(),
			true, true, 3);

	EtLayerView *layer_view = et_layer_view_init();
	if(NULL == layer_view){
		et_bug("");
		return -1;
	}
	if(0 > et_etaion_add_slot_change_state(
				slot_et_layer_view_from_etaion_change_state, NULL)){
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
				slot_et_etaion_from_mouse_action)){
		et_error("");
		return -1;
	}

	if(!et_pointing_manager_add_slot_mouse_action(
				slot_from_mouse_action_)){
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
				slot_et_pointing_manager_from_mouse_action, NULL)){
		et_error("");
		return -1;
	}


	if(NULL != args->input_filepath){
		EtDocId doc_id = _open_doc_new_from_file(args->input_filepath);
		if(doc_id < 0){
			et_error("");
			fprintf(stderr, "input_filepath can't open. :'%s'\n", args->input_filepath);
			usage();
			exit(EXIT_FAILURE);
		}
		et_debug("input_filepath success open.:'%s'", args->input_filepath);

		if(NULL != args->output_filepath){
			if(!_output_file_from_doc_id(args->output_filepath, doc_id)){
				et_error("output_filepath not success.:%s", args->output_filepath);
				usage();
				exit(EXIT_FAILURE);
			}
			exit (EXIT_SUCCESS);
		}
	}


	GThread* thread;
	thread = g_thread_new ("", _worker_func, NULL);
	if(NULL == thread){
		et_critical("");
		return -1;
	}

	char *window_title = g_strdup_printf("%s %s", "Vecterion", SHOW_VERSION);
	gtk_window_set_title(GTK_WINDOW(window), window_title);
	_pvui_app_set_style();
	gtk_widget_show_all(window);
	gtk_main();

	isStop = 1;
	g_thread_join(thread);

	return 0;
}

static EtDocId _open_doc_new_from_file(const char* filepath)
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
		vg_src = pv_io_new_from_file(filepath);
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
		PvElement *element_raster = pv_element_raster_new_from_filepath(filepath);
		if(NULL == element_raster){
			et_error("");
			goto finally;
		}
		if(! pv_element_append_child(element_parent, NULL, element_raster)){
			et_error("");
			goto finally;
		}

		PvElementRasterData *data = element_raster->data;
		vg_src->rect.w = gdk_pixbuf_get_width(data->pixbuf);
		vg_src->rect.h = gdk_pixbuf_get_height(data->pixbuf);
	}

	doc_id = _open_doc_new(vg_src);
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

static EtDocId _open_doc_new(PvVg *vg_src)
{
	return open_doc_new(vg_src);
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
	vg->rect.w = 1200;
	vg->rect.h = 800;

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

static bool _output_file_from_doc_id(const char *filepath, EtDocId doc_id)
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
		if(!_output_svg_from_doc_id(filepath, doc_id)){
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
		GdkPixbuf *pixbuf = pv_renderer_pixbuf_from_vg(vg, render_context, NULL, NULL);
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

static bool _output_svg_from_doc_id(const char *filepath, EtDocId doc_id)
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

static char *_get_dirpath_from_filepath(const char *filepath)
{
	if(NULL == filepath){
		return NULL;
	}

	char *dirpath = g_strdup(filepath);
	et_assert(dirpath);

	char *sep = strrchr(dirpath, '/');
	if(NULL == sep){
		g_free(dirpath);
		return NULL;
	}

	*sep = '\0';

	return dirpath;
}

/*! @return Acceps:new string of filepath(need g_free()). Cancel:NULL */
static char *_save_dialog_run(const char *dialog_title, const char *accept_button_title, const char *default_filepath)
{
	et_assert(default_filepath);

	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
	GtkWidget *dialog = gtk_file_chooser_dialog_new (dialog_title,
			self->window,
			action,
			_("_Cancel"),
			GTK_RESPONSE_CANCEL,
			accept_button_title,
			GTK_RESPONSE_ACCEPT,
			NULL);
	GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

	gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

	char *filename = strrchr(default_filepath, '/');
	if(NULL == filename){
		gtk_file_chooser_set_current_name (chooser, default_filepath);
	}else{
		char *dirpath = _get_dirpath_from_filepath(default_filepath);
		if(NULL != dirpath){
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

static gboolean _cb_menu_file_save(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Save:nothing document.");
		et_bug("%d\n", doc_id);
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
			char *next_filepath = pv_file_format_change_new_extension_from_filepath(filepath, "svg");
			g_free(filepath);
			filepath = next_filepath;
		}
	}

	if(NULL == filepath){
		// ** user select filepath
		filepath = _save_dialog_run("Save File", _("_Save"), _("untitled_document.svg"));

		if(NULL == filepath){
			et_debug("Cancel");
			goto finally;
		}
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
	if(!_output_svg_from_doc_id(filepath, doc_id)){
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

static gboolean _cb_menu_file_save_as(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Save:nothing document.");
		et_bug("%d\n", doc_id);
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
	filepath = _save_dialog_run("Save File", _("_Save"), tmp_filepath);

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

	if(!_output_svg_from_doc_id(filepath, doc_id)){
		_show_error_dialog("Save:'%s'", filepath);
		goto finally;
	}
	EtDocId dst_doc_id = _open_doc_new_from_file(filepath);
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

static gboolean _cb_menu_file_export(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Export:nothing document.");
		et_bug("%d\n", doc_id);
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

	filepath = _save_dialog_run("Export File", _("_Export"), prev_filepath);
	if(NULL == filepath){
		// cancel
		et_debug("Cancel:%s", prev_filepath);
		goto finally;
	}

	if(!_output_file_from_doc_id(filepath, doc_id)){
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

static gboolean _cb_menu_file_close(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d\n", doc_id);
		return false;
	}

	close_doc_from_id(doc_id, window_.status_bar);

	return false;
}

static gboolean _cb_menu_file_quit(gpointer data)
{
	_gui_quit();
	return FALSE;
}

static gboolean _cb_menu_edit_undo(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d\n", doc_id);
		return false;
	}

	if(!et_doc_undo_from_id(doc_id)){
		et_error("");
		return false;
	}

	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean _cb_menu_edit_redo(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d\n", doc_id);
		return false;
	}

	if(!et_doc_redo_from_id(doc_id)){
		et_error("");
		return false;
	}

	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean _cb_menu_edit_delete(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d\n", doc_id);
		return false;
	}

	if(!et_etaion_remove_delete_by_focusing(doc_id)){
		et_error("");
		return false;
	}

	et_doc_signal_update_from_id(doc_id);

	return false;
}

static gboolean _cb_menu_layer_new(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d\n", doc_id);
		return false;
	}

	et_etaion_append_new_layer(doc_id);

	return false;
}

static gboolean _cb_menu_layer_copy(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d\n", doc_id);
		return false;
	}

	et_etaion_copy_layer(doc_id);

	return false;
}

static gboolean _cb_menu_layer_new_child(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d\n", doc_id);
		return false;
	}

	et_etaion_append_new_layer_child(doc_id);

	return false;
}

static gboolean _cb_menu_layer_delete(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//_show_error_dialog("Close:nothing document.");
		//et_bug("%d\n", doc_id);
		return false;
	}

	et_etaion_remove_delete_layer(doc_id);

	return false;
}

static bool _gui_quit()
{
	EtDocId doc_id = -1;
	while(-1 != (doc_id = et_etaion_get_current_doc_id())){
		if(!close_doc_from_id(doc_id, window_.status_bar)){
			// Cancel
			et_debug("Quit: canceled.");
			return false;
		}
	}

	et_debug("Quit");
	gtk_main_quit();

	return true;
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
		EtDocId doc_id = _open_doc_new_from_file(filename);
		if(doc_id < 0){
			_show_error_dialog("Open:open error.:'%s'", filename);
		}
		g_free (filename);
	}

	gtk_widget_destroy (dialog);



	return false;
}

static bool _pv_element_append_near_first_parrent_layer(PvFocus *focus, PvElement *element)
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

PvElement *_new_element_from_filepath(const char *filepath)
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
		element = pv_element_raster_new_from_filepath(filepath);
	}

	if(NULL == element){
		et_warning("%s", filepath);
	}

	return element;
}

static gboolean _cb_menu_file_import(gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Import:nothing document.");
		et_bug("%d\n", doc_id);
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

		PvElement *import_element = _new_element_from_filepath(filename);
		if(NULL == import_element){
			_show_error_dialog("Import:open error.:%s", filename);
			et_debug("%d", doc_id);
			goto finally_1;
		}

		if(!_pv_element_append_near_first_parrent_layer(focus, import_element)){
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

static void _cb_menu_view_extent(GtkCheckMenuItem *menuitem, gpointer user_data)
{
	if(!et_etaion_set_is_extent_view(gtk_check_menu_item_get_active(menuitem))){
		et_error("");
	}
}

static bool _cb_menu_document_resize (GtkMenuItem *menuitem, gpointer user_data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Resize:nothing document.");
		et_bug("%d\n", doc_id);
		return false;
	}

	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	if(NULL == vg){
		_show_error_dialog("Resize:internal error.");
		et_debug("%d", doc_id);
		return false;
	}

	GtkWidget *dialog;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_dialog_new_with_buttons ("Resize Document",
			NULL,
			flags,
			"_OK",
			GTK_RESPONSE_ACCEPT,
			"_Cancel",
			GTK_RESPONSE_REJECT,
			NULL);

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
		_show_error_dialog("Select All:nothing document.");
		et_bug("%d\n", doc_id);
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
				_cb_menu_select_all_func_recurse_prev,
				&func_safr_data_pack,
				&error))
	{
		_show_error_dialog("Select All:internal error.");
		et_error("level:%d", error.level);
		return;
	}

	et_doc_signal_update_from_id(doc_id);
}

static void _cb_menu_select_none (GtkMenuItem *menuitem, gpointer user_data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Select:nothing document.");
		et_bug("%d\n", doc_id);
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

static void _cb_menu_select_invert (GtkMenuItem *menuitem, gpointer user_data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		_show_error_dialog("Select Invert:nothing document.");
		et_bug("%d\n", doc_id);
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

	int num = pv_general_get_parray_num((void **)focus->elements);
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
				_cb_menu_select_all_func_recurse_prev,
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

static void _cb_menu_help_about (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWindow *parent_window = NULL;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *dialog = gtk_message_dialog_new (parent_window,
			flags,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_CLOSE,
			"`%s`\n%s",
			VECTERION_FULLNAME,
			get_vecterion_build());
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

	// ** "/_File/_Import"
	menuitem = gtk_menu_item_new_with_label ("_Import");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_file_import), NULL);

	// ** "/_File/_Save (Ctrl+S)"
	menuitem = gtk_menu_item_new_with_label ("_Save");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_file_save), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// ** "/_File/Save _As (Ctrl+Shift+S)"
	menuitem = gtk_menu_item_new_with_label ("Save _As");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_file_save_as), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_s, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** "/_File/_Export (Ctrl+Shift+E)"
	menuitem = gtk_menu_item_new_with_label ("_Export");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_file_export), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_e, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** "/_File/_Close (Ctrl+W)"
	menuitem = gtk_menu_item_new_with_label ("_Close");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_file_close), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_w, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	// ** "/_File/_Quit (Ctrl+Q)"
	menuitem = gtk_menu_item_new_with_label ("_Quit");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_file_quit), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_q, (GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

static GtkWidget *_pv_get_menuitem_new_tree_of_edit(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	menuitem_root = gtk_menu_item_new_with_label ("_Edit");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "/_Edit/_Undo (Ctrl+Z)"
	menuitem = gtk_menu_item_new_with_label ("_Undo");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_edit_undo), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_z, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Edit/_Redo (Ctrl+Shift+Z)"
	menuitem = gtk_menu_item_new_with_label ("_Redo");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_edit_redo), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_z, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Edit/_Delete"
	menuitem = gtk_menu_item_new_with_label ("_Delete");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_edit_delete), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_Delete, 0, GTK_ACCEL_VISIBLE);

	return menuitem_root;
}

static GtkWidget *_pv_get_menuitem_new_tree_of_layer(GtkAccelGroup *accel_group){
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	menuitem_root = gtk_menu_item_new_with_label ("_Layer");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "/_Layer/_New Layer (Ctrl+Shift+N)"
	menuitem = gtk_menu_item_new_with_label ("_New Layer");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_layer_new), NULL);
	gtk_widget_add_accelerator (menuitem, "activate", accel_group,
			GDK_KEY_n, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

	// ** Accel to "/_Layer/_Copy Layer"
	menuitem = gtk_menu_item_new_with_label ("_Copy Layer");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_layer_copy), NULL);

	// ** Accel to "/_Layer/New Child Layer"
	menuitem = gtk_menu_item_new_with_label ("New Child Layer");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_layer_new_child), NULL);

	// ** Accel to "/_Layer/_Delete Layer"
	menuitem = gtk_menu_item_new_with_label ("_Delete Layer");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_layer_delete), NULL);

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

static GtkWidget *_pv_get_menuitem_new_tree_of_document(GtkAccelGroup *accel_group)
{
	GtkWidget *menuitem_root;
	GtkWidget *menuitem;
	GtkWidget *menu;

	menuitem_root = gtk_menu_item_new_with_mnemonic ("_Document");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem_root), TRUE);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_root), menu);

	// ** Accel to "Document > Resize Document
	menuitem = gtk_menu_item_new_with_mnemonic ("_Resize Document");
	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (menuitem), TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(menuitem, "activate", G_CALLBACK(_cb_menu_document_resize), NULL);

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

	menuitem = _pv_get_menuitem_new_tree_of_edit(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = _pv_get_menuitem_new_tree_of_layer(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = _pv_get_menuitem_new_tree_of_select(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = _pv_get_menuitem_new_tree_of_view(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = _pv_get_menuitem_new_tree_of_document(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menuitem = _new_tree_of_help(accel_group);
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	return true;
}

static bool slot_from_mouse_action_(EtDocId doc_id, EtMouseAction mouse_action)
{
	assert(self);

	char s[256];
	snprintf(s, sizeof(s),
			"%7.2f,%7.2f",
			mouse_action.point.x,
			mouse_action.point.y);
	gtk_statusbar_push(GTK_STATUSBAR(self->status_bar), 1, s);

	return true;
}

static void _show_error_dialog_impl(const char *message)
{
	et_assert(message);

	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *dialog = gtk_message_dialog_new (
			window_.window,
			flags,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			"%s",
			message);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	return;
}

