/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_LAYER_VIEW_H
#define include_ET_LAYER_VIEW_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc.h"
#include "pv_render_context.h"
#include "et_state.h"

struct EtLayerView;
typedef struct EtLayerView EtLayerView;

typedef void (*EtLayerViewSlotChange)(EtLayerView *layer_view, gpointer data);


EtLayerView *et_layer_view_init();
GtkWidget *et_layer_view_get_widget_frame();
bool et_layer_view_set_doc_id(EtDocId doc_id);
void et_layer_view_slot_from_doc_change(EtDoc *doc, gpointer data);
void slot_et_layer_view_from_etaion_change_state(EtState state, gpointer data);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_LAYER_VIEW_H
