#ifndef __ET_CANVAS_H__
#define __ET_CANVAS_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "et_mouse_action.h"
#include "pv_render_context.h"

struct _EtCanvas;
typedef struct _EtCanvas EtCanvas;

typedef void (*EtCanvasSlotChange)(EtCanvas *canvas, gpointer data);
typedef bool (*EtCanvasSlotMouseAction)(EtDocId id_doc, EtMouseAction mouse_action);

struct _EtCanvas{
	GtkWidget *widget; // Top widget pointer.
	GtkWidget *box;
	GtkWidget *box_infobar;
	GtkWidget *text_scale;
	GtkWidget *scroll;
	GtkWidget *event_box;
	GtkWidget *canvas;

	PvRenderContext render_context;

	EtDocId doc_id;
	GdkPixbuf *pixbuf_buffer;

	EtCanvasSlotChange slot_change;
	gpointer slot_change_data;
	EtCanvasSlotMouseAction slot_mouse_action;
	gpointer slot_mouse_action_data;
};

EtCanvas *et_canvas_new();
bool et_canvas_draw_pixbuf(EtCanvas *this, GdkPixbuf *pixbuf);
int et_canvas_set_slot_change(EtCanvas *this,
		EtCanvasSlotChange slot, gpointer data);
int et_canvas_set_slot_mouse_action(EtCanvas *this,
		EtCanvasSlotMouseAction slot, gpointer data);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_CANVAS_H__
