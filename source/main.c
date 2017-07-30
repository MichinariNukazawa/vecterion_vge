#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <stdbool.h>
#include <stdatomic.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "et_error.h"
#include "et_define.h"
#include "et_canvas_collection.h"
#include "et_renderer.h"
#include "et_pointing_manager.h"
#include "et_doc_manager.h"
#include "et_etaion.h"
#include "et_layer_view.h"
#include "et_tool_info.h"
#include "et_tool_panel.h"
#include "et_color_panel.h"
#include "et_stroke_panel.h"
#include "et_position_panel.h"
#include "pv_file_format.h"
#include "et_doc_relation.h"
#include "et_clipboard_manager.h"
#include "et_document_preference_dialog.h"
#include "et_menu.h"
#include "version.h"

static EtWindow window_;
static EtWindow *self = &window_;

static void et_app_set_style_();
static void init_gtk_builder_();
// static bool _debug_init();



static bool slot_from_mouse_action_(EtDocId, EtMouseAction);


static gboolean in_worker_func(gpointer data)
{
	static int count = 0;
	count++;
	// Gtk control.
	et_debug("work:%d", count);

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

static gboolean cb_key_press_(GtkWidget *widget, GdkEventKey * event, gpointer user_data)
{
	et_debug("keyval=0x%04x status=0x%04x",
			event->keyval, event->state/*, event->string*/);

	EtKeyAction ka = {
		.key = event->keyval,
		.action = EtKeyAction_Down,
		.state = event->state,
	};
	slot_et_etaion_from_key_action(ka);

	return FALSE;
}

static gboolean cb_delete_event_(GtkWidget *widget,
		GdkEvent  *event,
		gpointer   user_data)
{
	return !gui_quit_();
}


typedef struct{
	const char *input_filepath;
	const char *output_filepath;
	bool is_strict;
}EtArgs;

EtArgs EtArgs_Default = {
	NULL,
	NULL,
	false,
};

static void usage()
{
	fprintf(stderr,
			"usage: %s [-i input_filepath [-o output_filepath]]\n"
			" -i input_filepath		open file.\n"
			" 				default if svg format is extract pvvg structure in memory.\n"
			" -o output_filepath		output file and exit this app.\n"
			" 				depend -i option.\n"
			" 				file format from filename extension.\n"
			" -s				strict mode for input.\n"
			"				debug.\n"
			"				depend -i option.\n"
			" -h				show help.\n"
			,
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
	while (-1 != (opt = getopt(argc, argv, "i:o:sh"))) {
		switch (opt) {
			case 'i':
				args->input_filepath = optarg;
				break;
			case 'o':
				args->output_filepath = optarg;
				break;
			case 's':
				args->is_strict = true;
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

	if(args->is_strict){
		if(NULL == args->input_filepath){
			fprintf(stderr, "strict: input_filepath is not exist.\n");
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
		exit(EXIT_FAILURE);
	}

	self->vecterion_build = get_vecterion_build();

	static PvImageFileReadOption imageFileReadOption_;
	static PvImageFileReadOption *imageFileReadOption = &imageFileReadOption_;
	*imageFileReadOption = PvImageFileReadOption_Default;
	imageFileReadOption->is_strict = args->is_strict;

	// ** The etaion core modules initialize.
	EtEtaion *current_state = et_etaion_init();
	if(NULL == current_state){
		et_bug("");
		return -1;
	}

	et_etaion_set_application_base_dir_from_execute_path(argv[0]);

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

	EtClipboardManager *clipboard_manager = et_clipboard_manager_init();
	if(NULL == clipboard_manager){
		et_error("");
		return -1;
	}

	init_gtk_builder_();

	// ** window and container(box)s application base.
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request (window, 900, 700);
	self->window = GTK_WINDOW(window);

	GtkWidget *box_root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_container_add(GTK_CONTAINER(window), box_root);

	g_signal_connect(G_OBJECT(window), "key-press-event",
			G_CALLBACK(cb_key_press_), NULL);
	g_signal_connect(window, "delete-event",
			G_CALLBACK(cb_delete_event_), NULL);

	if(!et_tool_info_init(et_etaion_get_application_base_dir())){
		et_bug("");
		return -1;
	}

	// ** menu
	if(! init_menu(self, box_root)){
		et_bug("");
		return -1;
	}
	if(0 > et_etaion_add_slot_change_state(
				slot_et_menu_from_etaion_change_state, NULL)){
		et_bug("");
		return -1;
	}

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
	EtCanvasCollection *canvas_collection = et_canvas_collection_init();
	if(NULL == canvas_collection){
		et_bug("");
		return -1;
	}
	GtkWidget *cancol_widget = et_canvas_collection_get_widget_frame();
	gtk_paned_pack1 (GTK_PANED (hpaned), cancol_widget, TRUE, FALSE);

	et_etaion_set_widget_on_mouse_cursor(cancol_widget);

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
		EtDocId doc_id = open_doc_new_from_file_(args->input_filepath, imageFileReadOption);
		if(doc_id < 0){
			et_error("");
			fprintf(stderr, "input_filepath can't open. :'%s'\n", args->input_filepath);
			usage();
			exit(EXIT_FAILURE);
		}
		et_debug("input_filepath success open.:'%s'", args->input_filepath);

		if(NULL != args->output_filepath){
			if(!output_file_from_doc_id_(args->output_filepath, doc_id)){
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
	et_app_set_style_();
	gtk_widget_show_all(window);
	gtk_main();

	isStop = 1;
	g_thread_join(thread);

	return 0;
}

static void et_app_set_style_(){
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
			".ruler {\n"
			"   font-family: monospace;\n"
			//"   background-color: rgb (103, 103, 103);\n"
			"}\n"
			".canvas_widget {\n"
			"   padding: 0px;\n"
			"   border: 0px;\n"
			"   margin: 0px;\n"
			"}\n"
			"GtkTextView {\n"
			"   font-family: monospace;\n"
			"   font-size: 12px;\n"
			"}\n"
			"GtkSpinButton {\n"
			"   font-family: monospace;\n"
			"   padding: 4px;\n"
			//"   font-size: 12;\n"
			"}\n"
			"", -1, NULL);
	g_object_unref (provider);
}

static void init_gtk_builder_()
{
	{
		self->document_new_dialog_builder = gtk_builder_new();
		et_assert(self->document_new_dialog_builder);

		gchar *path = g_strdup_printf(
				"%s/resource/ui/document_new_dialog.glade",
				et_etaion_get_application_base_dir());

		GError* error = NULL;
		if ( !gtk_builder_add_from_file(self->document_new_dialog_builder, path, &error)){
			et_critical("Couldn't load document_new_dialog_builder file: %s", error->message);
			g_error_free(error);
			exit(1);
		}
		g_free(path);

		gtk_builder_connect_signals( self->document_new_dialog_builder, NULL);
	}

	self->document_preference_dialog_builder = pv_document_preference_dialog_init(
			et_etaion_get_application_base_dir());
	et_assert(self->document_preference_dialog_builder);
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

