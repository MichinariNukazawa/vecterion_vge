#ifndef __ET_CANVAS_H__
#define __ET_CANVAS_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

struct _EtCanvas;
typedef struct _EtCanvas EtCanvas;

struct _EtCanvas{
	GtkWidget *widget;
	GtkWidget *scroll;
	GtkWidget *event_box;
	GtkWidget *canvas;

	GdkPixbuf *pixbuf_buffer;
};

EtCanvas *et_canvas_new();
bool et_canvas_draw_pixbuf(EtCanvas *this, GdkPixbuf *pixbuf);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_CANVAS_H__
