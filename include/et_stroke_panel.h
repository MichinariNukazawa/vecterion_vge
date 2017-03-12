/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_STROKE_PANEL_H
#define include_ET_STROKE_PANEL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_stroke.h"
#include "et_state.h"
#include "et_doc_id.h"
#include "et_doc.h"

struct EtStrokePanel;
typedef struct EtStrokePanel EtStrokePanel;

EtStrokePanel *et_stroke_panel_init();
GtkWidget *et_stroke_panel_get_widget_frame();
PvStroke et_stroke_panel_get_stroke();

// @brief kick from change current document.
void slot_et_stroke_panel_from_etaion_change_state(EtState state, gpointer data);
void slot_et_stroke_panel_from_doc_change(EtDoc *doc, gpointer data);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_STROKE_PANEL_H
