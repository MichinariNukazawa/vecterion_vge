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

typedef void (*EtCanvasSlotChangeRenderContext)(EtCanvas *canvas, gpointer data);
typedef bool (*EtCanvasSignalMouseAction)(EtDocId id_doc, EtMouseAction mouse_action);

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

	EtCanvasSlotChangeRenderContext slot_rc;
	gpointer slot_rc_data;
	EtCanvasSignalMouseAction signal_mouse_action;
	gpointer signal_mouse_action_data;
};

EtCanvas *et_canvas_new();
bool et_canvas_draw_pixbuf(EtCanvas *this, GdkPixbuf *pixbuf);
int et_canvas_set_slot_change(EtCanvas *this,
		EtCanvasSlotChangeRenderContext slot, gpointer data);
int et_canvas_set_signal_mouse_action(EtCanvas *this,
		EtCanvasSignalMouseAction func, gpointer data);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_CANVAS_H__
