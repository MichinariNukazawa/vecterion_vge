#ifndef __ET_LAYER_VIEW_H__
#define __ET_LAYER_VIEW_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc.h"
#include "pv_render_context.h"
#include "et_state.h"

struct _EtLayerView;
typedef struct _EtLayerView EtLayerView;

typedef void (*EtLayerViewSlotChange)(EtLayerView *layer_view, gpointer data);


EtLayerView *et_layer_view_init();
GtkWidget *et_layer_view_get_widget_frame(EtLayerView *this);
bool et_layer_view_set_doc_id(EtLayerView *this, EtDocId doc_id);
void et_layer_view_slot_from_doc_change(EtDoc *doc, gpointer data);
void et_layer_view_slot_from_etaion_change_state(EtState state, gpointer data);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_LAYER_VIEW_H__
