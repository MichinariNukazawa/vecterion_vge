#ifndef __ET_CANVAS_H__
#define __ET_CANVAS_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

struct _EtRenderContext;
typedef struct _EtRenderContext EtRenderContext;

struct _EtRenderContext{
	double scale;
};

struct _EtCanvas;
typedef struct _EtCanvas EtCanvas;

typedef void (*EtCanvasUpdateCallback)(EtCanvas *canvas, gpointer data);

struct _EtCanvas{
	GtkWidget *widget;
	GtkWidget *scroll;
	GtkWidget *event_box;
	GtkWidget *canvas;

	EtRenderContext render_context;

	GdkPixbuf *pixbuf_buffer;
	EtCanvasUpdateCallback cb_update;
	gpointer cb_update_data;
};

EtCanvas *et_canvas_new();
bool et_canvas_draw_pixbuf(EtCanvas *this, GdkPixbuf *pixbuf);
int et_canvas_set_update_render_context(EtCanvas *this, EtCanvasUpdateCallback func, gpointer data);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_CANVAS_H__
