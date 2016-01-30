#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdbool.h>

#include "et_error.h"
#include "et_canvas.h"

void __pvui_app_set_style();
gboolean _buttonScroll(GtkWidget *widget, GdkEventScroll *event, gpointer data);

int main (int argc, char **argv){
	gtk_init(&argc, &argv);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request (window, 500,400);

	g_signal_connect(window, "delete-event",
			G_CALLBACK(gtk_main_quit), NULL);

	GtkWidget *box1;
	box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_container_add(GTK_CONTAINER(window), box1);

	const char *pathImageFile;
	pathImageFile = "./library/23.svg";

	GtkWidget *hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(box1), hpaned, true, true, 3);
	GtkWidget *frame1 = gtk_frame_new (NULL);
	GtkWidget *frame2 = gtk_frame_new (NULL);
	gtk_frame_set_label(GTK_FRAME (frame1), "test1");
	gtk_frame_set_label(GTK_FRAME (frame2), "test2");
	gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_IN);
	gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_IN);


	gtk_paned_pack1 (GTK_PANED (hpaned), frame1, TRUE, FALSE);
	gtk_paned_pack2 (GTK_PANED (hpaned), frame2, TRUE, FALSE);


	EtCanvas *canvas1 = et_canvas_new();
	if(!et_canvas_set_image_from_file(canvas1, pathImageFile)){
		et_critical("");
	}
	gtk_container_add(GTK_CONTAINER(frame1), canvas1->widget);

	EtCanvas *canvas2 = et_canvas_new();
	if(!et_canvas_set_image_from_file(canvas2, pathImageFile)){
		et_critical("");
	}
	gtk_container_add(GTK_CONTAINER(frame2), canvas2->widget);

	// __pvui_app_set_style();

	g_signal_connect(window, "scroll-event",
			G_CALLBACK(_buttonScroll), NULL);

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}


void __pvui_app_set_style(){
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


gboolean _buttonScroll(GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
	(void) data;

	switch(event->direction){
		case GDK_SCROLL_UP:
			et_debug("BUTTON SCROLL   UP: %ld\n", (long)widget);
			break;
		case GDK_SCROLL_DOWN:
			et_debug("BUTTON SCROLL DOWN: %ld\n", (long)widget);
			break;
		default:
			break;
	}

	return false;
}

