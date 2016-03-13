#ifndef include_ET_CANVAS_H
#define include_ET_CANVAS_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "et_doc.h"
#include "et_mouse_action.h"
#include "pv_render_context.h"

struct EtCanvas;
typedef struct EtCanvas EtCanvas;

typedef void (*EtCanvasSlotChange)(EtCanvas *canvas, gpointer data);
typedef bool (*EtCanvasSlotMouseAction)(EtDocId doc_id, EtMouseAction mouse_action);


EtCanvas *et_canvas_new();
GtkWidget *et_canvas_get_widget_frame(EtCanvas *this);
PvRenderContext et_canvas_get_render_context(EtCanvas *this, bool *isError);
EtDocId et_canvas_get_doc_id(EtCanvas *this);
bool et_canvas_set_doc_id(EtCanvas *this, EtDocId doc_id);
bool et_canvas_draw_pixbuf(EtCanvas *this, GdkPixbuf *pixbuf);
int et_canvas_set_slot_change(EtCanvas *this,
		EtCanvasSlotChange slot, gpointer data);
int et_canvas_set_slot_mouse_action(EtCanvas *this,
		EtCanvasSlotMouseAction slot, gpointer data);
void slot_et_canvas_from_doc_change(EtDoc *doc, gpointer data);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_CANVAS_H
