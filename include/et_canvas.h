#ifndef __ET_CANVAS_H__
#define __ET_CANVAS_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

struct _EtCanvas;
typedef struct _EtCanvas EtCanvas;

struct _EtCanvas{
	GdkPixbuf *pixbuf;

	GtkWidget *widget;
	GtkWidget *scroll;
	GtkWidget *event_box;
	GtkWidget *canvas;
};

EtCanvas *et_canvas_new();
bool et_canvas_set_image_from_file(EtCanvas *this, const char *filepath);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_CANVAS_H__
