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


EtCanvas *et_canvas_new_from_doc_id(EtDocId doc_id);
void et_canvas_delete(EtCanvas *);
GtkWidget *et_canvas_get_widget_frame(EtCanvas *self);
PvRenderContext et_canvas_get_render_context(EtCanvas *self, bool *isError);
EtDocId et_canvas_get_doc_id(EtCanvas *self);
bool et_canvas_set_doc_id(EtCanvas *self, EtDocId doc_id);
bool et_canvas_draw_pixbuf(EtCanvas *self, GdkPixbuf *pixbuf);
int et_canvas_set_slot_change(EtCanvas *self,
		EtCanvasSlotChange slot, gpointer data);
int et_canvas_set_slot_mouse_action(EtCanvas *self,
		EtCanvasSlotMouseAction slot, gpointer data);
void slot_et_canvas_from_doc_change(EtDoc *doc, gpointer data);

// ** use thumbnail.
void et_canvas_set_is_thumbnail(EtCanvas *self, bool is_thumbnail);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_CANVAS_H
